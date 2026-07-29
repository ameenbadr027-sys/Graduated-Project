
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "mbedtls/base64.h"


// WIFI

const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";


// APPS SCRIPT WEB APP URL


const char* APPS_SCRIPT_URL = "https://script.google.com/macros/s/XXXXXXXXXXXXXXXX/exec";

// Must match Code.gs
const char* ESP32_KEY = "esp32_cam_key_123";


// FLASH LED
// ESP32-CAM built-in flash LED is GPIO 4

#define FLASH_LED_PIN 4


// CAMERA PINS - AI Thinker ESP32-CAM

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// STATE

bool flashEnabled = false;
bool captureNowFlag = false;

unsigned long lastControlPollMs = 0;
const unsigned long CONTROL_POLL_MS = 5000;  // free-plan friendly


// HELPERS

String buildUrl(const String& action) {
  String url = String(APPS_SCRIPT_URL);
  url += "?action=" + action;
  url += "&key=" + String(ESP32_KEY);
  return url;
}

String httpGET(const String& url) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);

  if (!http.begin(client, url)) {
    Serial.println("HTTP begin failed");
    return "";
  }

  int code = http.GET();
  String payload = "";

  if (code > 0) {
    payload = http.getString();
  } else {
    Serial.printf("GET failed: %d\n", code);
  }

  http.end();
  return payload;
}

String httpPOSTText(const String& url, const String& body, const String& contentType) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);

  if (!http.begin(client, url)) {
    Serial.println("HTTP begin failed");
    return "";
  }

  http.addHeader("Content-Type", contentType);

  int code = http.POST(body);
  String response = "";

  if (code > 0) {
    response = http.getString();
  } else {
    Serial.printf("POST failed: %d\n", code);
  }

  http.end();
  return response;
}

String base64EncodeImage(const uint8_t* data, size_t len) {
  size_t outLen = 0;
  size_t encodedLen = 4 * ((len + 2) / 3) + 1; // +1 for null terminator

  uint8_t* out = (uint8_t*)malloc(encodedLen);
  if (!out) {
    Serial.println("Base64 malloc failed");
    return "";
  }

  int rc = mbedtls_base64_encode(out, encodedLen, &outLen, data, len);
  if (rc != 0) {
    Serial.printf("Base64 encode failed: %d\n", rc);
    free(out);
    return "";
  }

  out[outLen] = '\0';
  String encoded = String((char*)out);
  free(out);

  return encoded;
}

String jsonExtractValue(const String& json, const String& key) {
  String marker = "\"" + key + "\":\"";
  int start = json.indexOf(marker);
  if (start == -1) return "";

  start += marker.length();
  int end = json.indexOf("\"", start);
  if (end == -1) return "";

  String value = json.substring(start, end);
  value.replace("\\/", "/");
  return value;
}

bool jsonHasTrueField(const String& json, const String& key) {
  String pattern = "\"" + key + "\":true";
  return json.indexOf(pattern) != -1;
}

bool jsonHasFalseField(const String& json, const String& key) {
  String pattern = "\"" + key + "\":false";
  return json.indexOf(pattern) != -1;
}


// CAMERA INIT

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count     = 2;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size   = FRAMESIZE_QQVGA;
    config.jpeg_quality = 18;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
  }

  config.grab_mode = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    while (true) {
      delay(1000);
    }
  }
}

// =====================================================
// WIFI
// =====================================================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

// =====================================================
// READ CONTROL STATE FROM APPS SCRIPT
// =====================================================
void pollControls() {
  String response = httpGET(buildUrl("get"));
  if (response.length() == 0) return;

  String lower = response;
  lower.toLowerCase();

  if (jsonHasTrueField(lower, "flashenabled")) {
    flashEnabled = true;
  } else if (jsonHasFalseField(lower, "flashenabled")) {
    flashEnabled = false;
  }

  if (jsonHasTrueField(lower, "capturenow")) {
    captureNowFlag = true;
  } else if (jsonHasFalseField(lower, "capturenow")) {
    captureNowFlag = false;
  }

  Serial.print("flashEnabled: ");
  Serial.println(flashEnabled ? "true" : "false");

  Serial.print("captureNow: ");
  Serial.println(captureNowFlag ? "true" : "false");
}

// =====================================================
// UPLOAD IMAGE TO APPS SCRIPT
// =====================================================
String uploadSnapshotToAppsScript(const String& reason) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }

  if (flashEnabled) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(150);
  }

  String b64 = base64EncodeImage(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (flashEnabled) {
    delay(50);
    digitalWrite(FLASH_LED_PIN, LOW);
  }

  if (b64.length() == 0) {
    Serial.println("Base64 conversion failed");
    return "";
  }

  String url = String(APPS_SCRIPT_URL) +
               "?action=upload" +
               "&key=" + String(ESP32_KEY) +
               "&reason=" + reason;

  String response = httpPOSTText(url, b64, "text/plain");

  Serial.println("Upload response:");
  Serial.println(response);

  String imageUrl = jsonExtractValue(response, "imageUrl");
  return imageUrl;
}

void captureAndUpload(const String& reason) {
  Serial.print("Capturing image for: ");
  Serial.println(reason);

  String imageUrl = uploadSnapshotToAppsScript(reason);

  if (imageUrl.length() > 0) {
    Serial.println("Image URL:");
    Serial.println(imageUrl);
  } else {
    Serial.println("Upload failed");
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  initCamera();
  connectWiFi();

  pollControls();
}

// =====================================================
// LOOP
// =====================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Poll dashboard controls only at interval
  if (millis() - lastControlPollMs >= CONTROL_POLL_MS) {
    lastControlPollMs = millis();
    pollControls();
  }

  // Capture request from dashboard
  if (captureNowFlag) {
    captureNowFlag = false;
    captureAndUpload("manual");
  }

  delay(50);
}