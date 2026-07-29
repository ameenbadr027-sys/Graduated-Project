/*
  ================================================================================
  EOS - Smart Assistive Glasses for the Visually Impaired
  ESP32-CAM Firmware (Production-Ready Prototype)
  --------------------------------------------------------------------------------
  Architecture preserved from the original prototype:
    - Wi-Fi connectivity
    - Camera capture -> AI inference server (HTTP POST)
    - I2S audio output (placeholder for future TTS)
    - Local WebServer with REST-style status endpoints
    - Serial "CAPTURE" command trigger

  What changed vs. the prototype (see task list for full rationale):
    - Robust init/error handling everywhere (camera, WiFi, I2S, HTTP)
    - PSRAM-aware camera configuration
    - Non-blocking WiFi connect with timeout + auto-reconnect via events
    - HTTP client with timeout + retry + full status-code handling
    - AI response parsed with ArduinoJson into a proper AIResult struct
    - Heap/PSRAM/fragmentation diagnostics, printed periodically
    - Structured [INFO]/[WARN]/[ERROR]/[DEBUG] logging
    - New REST endpoints: /status, /memory, /capture, /health (all JSON)
    - All magic numbers centralized into one CONFIG section

  Dependencies (install via Arduino Library Manager):
    - ArduinoJson (v6.x or v7.x)
    - ESP32 board package (esp_camera, WebServer, WiFi, HTTPClient are bundled)

  Kept intentionally simple:
    - Single file, no extra classes, no RTOS task split.
    - Same overall program flow as the original prototype.
  ================================================================================
*/

#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include <HTTPClient.h>
#include "driver/i2s.h"
#include <ArduinoJson.h>
#include "esp_heap_caps.h"

// ================================================================================
// CONFIGURATION  (all tunables live here - avoid magic numbers elsewhere)
// ================================================================================
namespace Config {

  // ---------- Wi-Fi ----------
  constexpr const char* WIFI_SSID       = "0%";
  constexpr const char* WIFI_PASSWORD   = "20402022100532#";
  constexpr uint32_t    WIFI_CONNECT_TIMEOUT_MS   = 15000;  // give up after this long
  constexpr uint32_t    WIFI_RECONNECT_INTERVAL_MS = 5000;  // retry interval once dropped
  constexpr uint32_t    WIFI_BLINK_INTERVAL_MS     = 300;   // LED blink while connecting

  // ---------- AI Server ----------
  constexpr const char* AI_SERVER_URL   = "http://YOUR_AI_MODEL_SERVER_IP/predict";
  constexpr uint32_t    HTTP_TIMEOUT_MS         = 8000;   // per-request timeout
  constexpr uint8_t     HTTP_MAX_RETRIES        = 3;      // total attempts = retries
  constexpr uint32_t    HTTP_RETRY_BACKOFF_MS   = 500;    // delay between retries

  // ---------- LED ----------
  constexpr int LED_PIN = 2;

  // ---------- I2S ----------
  constexpr int      I2S_BCLK_PIN   = 14;
  constexpr int      I2S_WS_PIN     = 13;
  constexpr int      I2S_DOUT_PIN   = 12;
  constexpr uint32_t I2S_SAMPLE_RATE = 16000;
  constexpr int       I2S_DMA_BUF_COUNT = 8;
  constexpr int       I2S_DMA_BUF_LEN   = 64;

  // ---------- Camera pins (unchanged from original wiring) ----------
  constexpr int PWDN_GPIO_NUM    = -1;
  constexpr int RESET_GPIO_NUM   = -1;
  constexpr int XCLK_GPIO_NUM    = 15;
  constexpr int SIOD_GPIO_NUM    = 4;
  constexpr int SIOC_GPIO_NUM    = 5;

  constexpr int Y9_GPIO_NUM      = 16;
  constexpr int Y8_GPIO_NUM      = 17;
  constexpr int Y7_GPIO_NUM      = 18;
  constexpr int Y6_GPIO_NUM      = 19;
  constexpr int Y5_GPIO_NUM      = 20;
  constexpr int Y4_GPIO_NUM      = 21;
  constexpr int Y3_GPIO_NUM      = 38;
  constexpr int Y2_GPIO_NUM      = 37;
  constexpr int VSYNC_GPIO_NUM   = 36;
  constexpr int HREF_GPIO_NUM    = 35;
  constexpr int PCLK_GPIO_NUM    = 34;

  constexpr uint32_t XCLK_FREQ_HZ = 20000000;

  // Frame size / quality depending on PSRAM availability
  constexpr framesize_t FRAME_SIZE_WITH_PSRAM    = FRAMESIZE_SVGA; // 800x600
  constexpr framesize_t FRAME_SIZE_WITHOUT_PSRAM = FRAMESIZE_QVGA; // 320x240
  constexpr int JPEG_QUALITY_WITH_PSRAM    = 10; // lower number = higher quality
  constexpr int JPEG_QUALITY_WITHOUT_PSRAM = 12;
  constexpr int FB_COUNT_WITH_PSRAM    = 2;
  constexpr int FB_COUNT_WITHOUT_PSRAM = 1;

  constexpr uint8_t CAMERA_INIT_MAX_RETRIES = 3;
  constexpr uint8_t CAMERA_CAPTURE_MAX_RETRIES = 2;

  // ---------- System ----------
  constexpr uint32_t HEARTBEAT_LED_INTERVAL_MS = 1000;
  constexpr uint32_t MEMORY_REPORT_INTERVAL_MS = 30000; // periodic diagnostics
  constexpr uint32_t WEB_SERVER_PORT = 80;

} // namespace Config

// ================================================================================
// LOGGING  -  structured [INFO] / [WARNING] / [ERROR] / [DEBUG] output
// ================================================================================
enum class LogLevel : uint8_t { DEBUG, INFO, WARNING, ERROR };

// Centralized log function. Keeps Serial.println() usage out of the rest of
// the code and makes it trivial to redirect logs (e.g. to a file) later.
void logMsg(LogLevel level, const char* tag, const String& message) {
  const char* levelStr = "INFO";
  switch (level) {
    case LogLevel::DEBUG:   levelStr = "DEBUG";   break;
    case LogLevel::INFO:    levelStr = "INFO";    break;
    case LogLevel::WARNING: levelStr = "WARNING"; break;
    case LogLevel::ERROR:   levelStr = "ERROR";   break;
  }
  Serial.printf("[%s][%s] %s\n", levelStr, tag, message.c_str());
}

#define LOG_DEBUG(tag, msg) logMsg(LogLevel::DEBUG, tag, msg)
#define LOG_INFO(tag, msg)  logMsg(LogLevel::INFO, tag, msg)
#define LOG_WARN(tag, msg)  logMsg(LogLevel::WARNING, tag, msg)
#define LOG_ERROR(tag, msg) logMsg(LogLevel::ERROR, tag, msg)

// ================================================================================
// GLOBAL STATE
// ================================================================================
WebServer server(Config::WEB_SERVER_PORT);

bool g_cameraReady   = false;
bool g_i2sReady      = false;
bool g_wifiConnected = false;

unsigned long g_lastWifiRetryAttempt = 0;
unsigned long g_lastHeartbeat        = 0;
unsigned long g_lastMemoryReport     = 0;

// Result of the last AI inference, parsed from JSON. Kept as a simple POD
// struct rather than a class to keep the project easy to follow.
struct AIResult {
  bool   valid       = false;   // true only if parsing + fields succeeded
  String label       = "";
  float  confidence  = 0.0f;
  float  distance    = 0.0f;    // meters (or server-defined unit)
};

AIResult g_lastAIResult; // last known result, exposed via /status

// ================================================================================
// FORWARD DECLARATIONS
// ================================================================================
void connectWiFi();
void onWiFiEvent(WiFiEvent_t event);
bool initCamera();
bool recoverCamera();
bool initI2S();
void playAudio(const uint8_t* data, size_t len);
bool captureAndSendToAI(AIResult& outResult);
void printMemoryDiagnostics();
void setupWebServer();
void handleRoot();
void handleStatus();
void handleMemory();
void handleCapture();
void handleHealth();
void handleNotFound();

// ================================================================================
// WI-FI
// ================================================================================

// WiFi event callback - handles disconnects/reconnects and diagnostics.
// Registered once in setup() via WiFi.onEvent().
void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      LOG_INFO("WiFi", "Station connected to AP.");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      g_wifiConnected = true;
      LOG_INFO("WiFi", "Got IP address: " + WiFi.localIP().toString());
      LOG_INFO("WiFi", "RSSI: " + String(WiFi.RSSI()) + " dBm");
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      if (g_wifiConnected) {
        LOG_WARN("WiFi", "Connection lost. Will attempt reconnect.");
      }
      g_wifiConnected = false;
      break;

    default:
      break;
  }
}

// Blocking connect with a hard timeout so we never hang forever on boot.
// If it fails, we continue booting anyway - reconnection is retried in loop().
void connectWiFi() {
  LOG_INFO("WiFi", "Connecting to SSID: " + String(Config::WIFI_SSID));

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(onWiFiEvent);
  WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  unsigned long lastBlink = 0;
  bool ledState = false;

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttempt > Config::WIFI_CONNECT_TIMEOUT_MS) {
      LOG_ERROR("WiFi", "Connection timed out after " +
                 String(Config::WIFI_CONNECT_TIMEOUT_MS) + " ms. Continuing without WiFi.");
      return;
    }

    if (millis() - lastBlink > Config::WIFI_BLINK_INTERVAL_MS) {
      ledState = !ledState;
      digitalWrite(Config::LED_PIN, ledState);
      lastBlink = millis();
    }
    delay(10); // small yield, avoids busy-looping the CPU
  }

  g_wifiConnected = true;
  LOG_INFO("WiFi", "Connected. IP: " + WiFi.localIP().toString() +
                    " | RSSI: " + String(WiFi.RSSI()) + " dBm");
}

// Called every loop() iteration. Non-blocking: only attempts a reconnect
// every WIFI_RECONNECT_INTERVAL_MS, and never halts the rest of the system.
void maintainWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  g_wifiConnected = false;

  if (millis() - g_lastWifiRetryAttempt < Config::WIFI_RECONNECT_INTERVAL_MS) {
    return;
  }
  g_lastWifiRetryAttempt = millis();

  LOG_WARN("WiFi", "Not connected. Attempting reconnect...");
  WiFi.disconnect();
  WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
}

// ================================================================================
// CAMERA
// ================================================================================

// Builds a camera_config_t based on whether PSRAM is available, then
// initializes the camera driver. Retries a few times before giving up.
bool initCamera() {
  bool hasPsram = psramFound();
  LOG_INFO("Camera", hasPsram ? "PSRAM detected - using higher quality profile."
                               : "No PSRAM detected - using conservative profile.");

  camera_config_t config = {};
  config.pin_pwdn     = Config::PWDN_GPIO_NUM;
  config.pin_reset    = Config::RESET_GPIO_NUM;
  config.pin_xclk     = Config::XCLK_GPIO_NUM;
  config.pin_sscb_sda = Config::SIOD_GPIO_NUM;
  config.pin_sscb_scl = Config::SIOC_GPIO_NUM;

  config.pin_d7 = Config::Y9_GPIO_NUM;
  config.pin_d6 = Config::Y8_GPIO_NUM;
  config.pin_d5 = Config::Y7_GPIO_NUM;
  config.pin_d4 = Config::Y6_GPIO_NUM;
  config.pin_d3 = Config::Y5_GPIO_NUM;
  config.pin_d2 = Config::Y4_GPIO_NUM;
  config.pin_d1 = Config::Y3_GPIO_NUM;
  config.pin_d0 = Config::Y2_GPIO_NUM;

  config.pin_vsync = Config::VSYNC_GPIO_NUM;
  config.pin_href  = Config::HREF_GPIO_NUM;
  config.pin_pclk  = Config::PCLK_GPIO_NUM;

  config.xclk_freq_hz = Config::XCLK_FREQ_HZ;
  config.ledc_timer   = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.pixel_format = PIXFORMAT_JPEG;

  // Dynamic configuration depending on PSRAM presence.
  config.frame_size   = hasPsram ? Config::FRAME_SIZE_WITH_PSRAM
                                  : Config::FRAME_SIZE_WITHOUT_PSRAM;
  config.jpeg_quality  = hasPsram ? Config::JPEG_QUALITY_WITH_PSRAM
                                   : Config::JPEG_QUALITY_WITHOUT_PSRAM;
  config.fb_count      = hasPsram ? Config::FB_COUNT_WITH_PSRAM
                                   : Config::FB_COUNT_WITHOUT_PSRAM;
  config.fb_location   = hasPsram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  config.grab_mode     = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = ESP_FAIL;
  for (uint8_t attempt = 1; attempt <= Config::CAMERA_INIT_MAX_RETRIES; attempt++) {
    err = esp_camera_init(&config);
    if (err == ESP_OK) {
      LOG_INFO("Camera", "Initialized successfully (attempt " + String(attempt) + ").");
      g_cameraReady = true;
      return true;
    }

    LOG_ERROR("Camera", "esp_camera_init failed (0x" + String(err, HEX) +
               ") on attempt " + String(attempt) + "/" +
               String(Config::CAMERA_INIT_MAX_RETRIES));
    delay(300); // brief settle time before retrying
  }

  g_cameraReady = false;
  LOG_ERROR("Camera", "Camera initialization failed permanently.");
  return false;
}

// Deinitializes and reinitializes the camera driver. Used when a capture
// fails repeatedly, which usually indicates the sensor got into a bad state.
bool recoverCamera() {
  LOG_WARN("Camera", "Attempting camera recovery (deinit + reinit)...");
  esp_err_t deinitErr = esp_camera_deinit();
  if (deinitErr != ESP_OK) {
    LOG_ERROR("Camera", "esp_camera_deinit failed (0x" + String(deinitErr, HEX) + ").");
  }
  delay(200);
  return initCamera();
}

// ================================================================================
// I2S AUDIO
// ================================================================================

// Initializes I2S for future TTS/audio-feedback output. Validates both the
// driver install and the pin configuration steps instead of ignoring errors.
bool initI2S() {
  i2s_config_t i2sConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = Config::I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = Config::I2S_DMA_BUF_COUNT,
    .dma_buf_len = Config::I2S_DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = true
  };

  esp_err_t installErr = i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
  if (installErr != ESP_OK) {
    LOG_ERROR("I2S", "i2s_driver_install failed (0x" + String(installErr, HEX) + ").");
    g_i2sReady = false;
    return false;
  }

  i2s_pin_config_t pinConfig = {};
  pinConfig.bck_io_num   = Config::I2S_BCLK_PIN;
  pinConfig.ws_io_num    = Config::I2S_WS_PIN;
  pinConfig.data_out_num = Config::I2S_DOUT_PIN;
  pinConfig.data_in_num  = I2S_PIN_NO_CHANGE;

  esp_err_t pinErr = i2s_set_pin(I2S_NUM_0, &pinConfig);
  if (pinErr != ESP_OK) {
    LOG_ERROR("I2S", "i2s_set_pin failed (0x" + String(pinErr, HEX) + ").");
    i2s_driver_uninstall(I2S_NUM_0);
    g_i2sReady = false;
    return false;
  }

  LOG_INFO("I2S", "I2S driver initialized successfully.");
  g_i2sReady = true;
  return true;
}

// Placeholder for future TTS playback. Kept as a stub with the correct
// signature (raw PCM buffer + length) so the AI-response pipeline can call
// it directly once a TTS/audio-decoding source is wired in.
void playAudio(const uint8_t* data, size_t len) {
  if (!g_i2sReady) {
    LOG_WARN("I2S", "playAudio() called but I2S is not ready. Skipping.");
    return;
  }
  if (data == nullptr || len == 0) {
    LOG_WARN("I2S", "playAudio() called with empty buffer.");
    return;
  }

  size_t bytesWritten = 0;
  esp_err_t err = i2s_write(I2S_NUM_0, data, len, &bytesWritten, portMAX_DELAY);
  if (err != ESP_OK) {
    LOG_ERROR("I2S", "i2s_write failed (0x" + String(err, HEX) + ").");
    return;
  }
  LOG_DEBUG("I2S", "Wrote " + String(bytesWritten) + "/" + String(len) + " bytes.");
}

// ================================================================================
// AI COMMUNICATION  (capture -> HTTP POST -> JSON parse)
// ================================================================================

// Captures a JPEG frame and posts it to the AI server. Retries on transient
// failures (network errors, 5xx) and recovers the camera if the frame buffer
// grab itself fails. Parses the JSON response into AIResult using ArduinoJson,
// guarding against malformed payloads.
bool captureAndSendToAI(AIResult& outResult) {
  outResult = AIResult(); // reset to defaults

  if (!g_cameraReady) {
    LOG_ERROR("AI", "Camera not ready - cannot capture.");
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    LOG_ERROR("AI", "WiFi not connected - cannot reach AI server.");
    return false;
  }

  // --- Capture frame, with a small retry/recovery loop ---
  camera_fb_t* fb = nullptr;
  for (uint8_t attempt = 1; attempt <= Config::CAMERA_CAPTURE_MAX_RETRIES; attempt++) {
    fb = esp_camera_fb_get();
    if (fb) break;

    LOG_WARN("Camera", "Capture failed on attempt " + String(attempt) + "/" +
             String(Config::CAMERA_CAPTURE_MAX_RETRIES));
    if (attempt < Config::CAMERA_CAPTURE_MAX_RETRIES) {
      recoverCamera();
    }
  }

  if (!fb) {
    LOG_ERROR("Camera", "Capture failed permanently after recovery attempts.");
    return false;
  }

  LOG_DEBUG("Camera", "Captured frame: " + String(fb->len) + " bytes.");

  // --- Send to AI server with timeout + retry ---
  bool success = false;
  String responseBody;
  int httpCode = -1;

  for (uint8_t attempt = 1; attempt <= Config::HTTP_MAX_RETRIES; attempt++) {
    HTTPClient http;
    http.setTimeout(Config::HTTP_TIMEOUT_MS);

    if (!http.begin(Config::AI_SERVER_URL)) {
      LOG_ERROR("HTTP", "http.begin() failed - invalid URL: " + String(Config::AI_SERVER_URL));
      break; // config error, retrying won't help
    }
    http.addHeader("Content-Type", "image/jpeg");

    httpCode = http.POST(fb->buf, fb->len);

    if (httpCode > 0) {
      responseBody = http.getString();

      if (httpCode == HTTP_CODE_OK) {
        success = true;
        http.end();
        break; // done, exit retry loop
      } else if (httpCode >= 500) {
        LOG_WARN("HTTP", "Server error " + String(httpCode) +
                 " on attempt " + String(attempt) + "/" + String(Config::HTTP_MAX_RETRIES));
      } else if (httpCode == HTTP_CODE_TOO_MANY_REQUESTS) {
        LOG_WARN("HTTP", "Rate limited (429) on attempt " + String(attempt));
      } else {
        // 4xx (excluding 429) is a client-side/config problem - retrying won't fix it.
        LOG_ERROR("HTTP", "Non-retryable HTTP status: " + String(httpCode) +
                  " | body: " + responseBody);
        http.end();
        break;
      }
    } else {
      // Negative return = connection-level error (timeout, DNS, refused, etc.)
      LOG_WARN("HTTP", "Request failed: " + http.errorToString(httpCode) +
               " (attempt " + String(attempt) + "/" + String(Config::HTTP_MAX_RETRIES) + ")");
    }

    http.end(); // always release the connection - avoids leaking sockets/memory
    if (attempt < Config::HTTP_MAX_RETRIES) {
      delay(Config::HTTP_RETRY_BACKOFF_MS);
    }
  }

  esp_camera_fb_return(fb); // always return the frame buffer, even on failure

  if (!success) {
    LOG_ERROR("AI", "All HTTP attempts failed. Last status: " + String(httpCode));
    return false;
  }

  // --- Parse JSON response safely ---
  // Expected shape: {"label":"person","confidence":0.98,"distance":1.2}
  JsonDocument doc; // ArduinoJson v7 style; auto-sized, no manual capacity math
  DeserializationError jsonErr = deserializeJson(doc, responseBody);

  if (jsonErr) {
    LOG_ERROR("AI", "Malformed JSON from AI server: " + String(jsonErr.c_str()));
    return false;
  }

  if (!doc["label"].is<const char*>() || !doc["confidence"].is<float>()) {
    LOG_ERROR("AI", "AI response missing required fields (label/confidence).");
    return false;
  }

  outResult.label      = doc["label"].as<String>();
  outResult.confidence = doc["confidence"].as<float>();
  outResult.distance    = doc["distance"] | 0.0f; // optional field, default 0
  outResult.valid       = true;

  LOG_INFO("AI", "Detected '" + outResult.label + "' (confidence=" +
           String(outResult.confidence, 2) + ", distance=" +
           String(outResult.distance, 2) + "m)");

  // Hand off to audio feedback pipeline (currently a stub - see playAudio()).
  // Real TTS/audio buffer would be generated/fetched here in the future.
  if (outResult.confidence > 0.0f) {
    // NOTE: no real PCM data yet - left as an integration point.
    // playAudio(pcmBuffer, pcmLength);
  }

  return true;
}

// ================================================================================
// MEMORY DIAGNOSTICS
// ================================================================================

// Prints heap/PSRAM diagnostics. Fragmentation is estimated as
// 1 - (largest_free_block / total_free_heap), expressed as a percentage.
void printMemoryDiagnostics() {
  uint32_t freeHeap        = ESP.getFreeHeap();
  uint32_t largestFreeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  uint32_t freePsram       = psramFound() ? ESP.getFreePsram() : 0;

  float fragmentation = 0.0f;
  if (freeHeap > 0) {
    fragmentation = 100.0f * (1.0f - ((float)largestFreeBlock / (float)freeHeap));
  }

  LOG_INFO("Memory", "Free Heap: " + String(freeHeap) + " bytes | " +
           "Largest Block: " + String(largestFreeBlock) + " bytes | " +
           "Free PSRAM: " + String(freePsram) + " bytes | " +
           "Fragmentation: " + String(fragmentation, 1) + "%");
}

// Builds the same diagnostics as a JSON object (used by /memory endpoint).
void buildMemoryJson(JsonDocument& doc) {
  uint32_t freeHeap        = ESP.getFreeHeap();
  uint32_t largestFreeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  uint32_t freePsram       = psramFound() ? ESP.getFreePsram() : 0;
  uint32_t totalPsram      = psramFound() ? ESP.getPsramSize() : 0;

  float fragmentation = 0.0f;
  if (freeHeap > 0) {
    fragmentation = 100.0f * (1.0f - ((float)largestFreeBlock / (float)freeHeap));
  }

  doc["free_heap_bytes"]        = freeHeap;
  doc["largest_free_block_bytes"] = largestFreeBlock;
  doc["free_psram_bytes"]       = freePsram;
  doc["total_psram_bytes"]      = totalPsram;
  doc["fragmentation_percent"]  = round(fragmentation * 10) / 10.0;
  doc["psram_present"]          = psramFound();
}

// ================================================================================
// WEB SERVER  -  REST endpoints, all returning JSON
// ================================================================================

void handleRoot() {
  server.send(200, "text/html",
    "<h1>EOS Smart Glasses - System OK</h1>"
    "<p>Endpoints: /status /memory /capture /health</p>");
}

// GET /status - overall system + last AI result
void handleStatus() {
  JsonDocument doc;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["ip_address"]     = WiFi.localIP().toString();
  doc["rssi_dbm"]       = WiFi.RSSI();
  doc["camera_ready"]   = g_cameraReady;
  doc["i2s_ready"]      = g_i2sReady;
  doc["uptime_ms"]      = millis();

  JsonObject lastResult = doc["last_ai_result"].to<JsonObject>();
  lastResult["valid"]      = g_lastAIResult.valid;
  lastResult["label"]      = g_lastAIResult.label;
  lastResult["confidence"] = g_lastAIResult.confidence;
  lastResult["distance"]   = g_lastAIResult.distance;

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// GET /memory - heap/PSRAM diagnostics
void handleMemory() {
  JsonDocument doc;
  buildMemoryJson(doc);

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// GET /capture - triggers an immediate capture + AI inference, returns result
void handleCapture() {
  AIResult result;
  bool ok = captureAndSendToAI(result);

  JsonDocument doc;
  doc["success"] = ok;
  if (ok) {
    g_lastAIResult = result; // cache for /status
    doc["label"]      = result.label;
    doc["confidence"] = result.confidence;
    doc["distance"]   = result.distance;
  } else {
    doc["error"] = "capture_or_inference_failed";
  }

  String out;
  serializeJson(doc, out);
  server.send(ok ? 200 : 502, "application/json", out);
}

// GET /health - lightweight liveness probe for monitoring
void handleHealth() {
  bool healthy = g_cameraReady && (WiFi.status() == WL_CONNECTED);

  JsonDocument doc;
  doc["status"] = healthy ? "ok" : "degraded";
  doc["camera_ready"] = g_cameraReady;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["i2s_ready"] = g_i2sReady;

  String out;
  serializeJson(doc, out);
  server.send(healthy ? 200 : 503, "application/json", out);
}

void handleNotFound() {
  JsonDocument doc;
  doc["error"] = "not_found";
  doc["path"]  = server.uri();

  String out;
  serializeJson(doc, out);
  server.send(404, "application/json", out);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/memory", HTTP_GET, handleMemory);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/health", HTTP_GET, handleHealth);
  server.onNotFound(handleNotFound);
  server.begin();
  LOG_INFO("Web", "REST server started on port " + String(Config::WEB_SERVER_PORT));
}

// ================================================================================
// SETUP
// ================================================================================
void setup() {
  Serial.begin(115200);
  delay(200); // let the serial monitor attach
  LOG_INFO("System", "EOS Smart Glasses booting...");

  pinMode(Config::LED_PIN, OUTPUT);
  digitalWrite(Config::LED_PIN, LOW);

  // --- WiFi ---
  connectWiFi();

  // --- Web server (starts even if WiFi failed - endpoints will just be
  //     unreachable until connectivity returns) ---
  setupWebServer();

  // --- Camera ---
  if (!initCamera()) {
    LOG_ERROR("System", "Camera unavailable. AI capture features will be disabled "
                          "until recovered.");
  }

  // --- I2S ---
  if (!initI2S()) {
    LOG_ERROR("System", "I2S unavailable. Audio feedback will be disabled.");
  }

  printMemoryDiagnostics();
  LOG_INFO("System", "Setup complete.");
}

// ================================================================================
// LOOP
// ================================================================================
void loop() {
  server.handleClient();
  maintainWiFiConnection();

  // --- Heartbeat LED (only used as a "system alive" indicator; connection
  //     status is now handled by decoupled logic above) ---
  if (millis() - g_lastHeartbeat > Config::HEARTBEAT_LED_INTERVAL_MS) {
    digitalWrite(Config::LED_PIN, !digitalRead(Config::LED_PIN));
    g_lastHeartbeat = millis();
  }

  // --- Periodic memory diagnostics ---
  if (millis() - g_lastMemoryReport > Config::MEMORY_REPORT_INTERVAL_MS) {
    printMemoryDiagnostics();
    g_lastMemoryReport = millis();
  }

  // --- Serial command interface (preserved from the original prototype) ---
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "CAPTURE") {
      AIResult result;
      if (captureAndSendToAI(result)) {
        g_lastAIResult = result;
      }
    } else if (cmd == "STATUS") {
      printMemoryDiagnostics();
      LOG_INFO("System", String("WiFi: ") + (g_wifiConnected ? "connected" : "disconnected") +
               " | Camera: " + (g_cameraReady ? "ready" : "not ready") +
               " | I2S: " + (g_i2sReady ? "ready" : "not ready"));
    } else if (cmd.length() > 0) {
      LOG_WARN("System", "Unknown serial command: " + cmd);
    }
  }
}
