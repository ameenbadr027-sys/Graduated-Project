# EOS — Smart Assistive Glasses for the Visually Impaired

**Hardware Assembly & Wiring Guide**

This document covers the physical hardware setup for the EOS project: required components, wiring diagrams, boot-up steps, and a breakdown of every file in this repository.

---

## 1. Project Overview

The system is made of **two physically separate microcontrollers** that communicate over Serial (UART):

| Device | Role | Responsible Sketch |
|---|---|---|
| Helper MCU (Arduino **Nano**) | Measures distance with an ultrasonic sensor and triggers a local buzzer alert; sends a command over Serial to the camera board | `EOS ARDUIONO/EOS_Obstacle_Detection.ino` |
| ESP32-CAM (**AI-Thinker** module) | Camera + Wi-Fi connectivity + sends captured frames to the AI inference server + audio output (I2S) + REST status server | `EOS ESP32CAM/EOS_Object_Detection.ino` |

There are also two alternative sketches for the ESP32-CAM, kept for diagnostics / a different deployment mode — see [Section 6](#6-which-sketch-should-i-actually-use):

- `EOS_Stream.ino` + `app_httpd.cpp` + `camera_index.h` — live camera stream, based on Espressif's official `CameraWebServer`.
- `EOS_Cloud_Control.ino` — **new**: cloud/dashboard control mode via Google Apps Script (fully explained in [Section 3.6](#36-new-eos_cloud_controlino--cloud-dashboard-mode)).

> [!WARNING]
> Only ever flash **one** of `EOS_Object_Detection.ino`, `EOS_Stream.ino`, or `EOS_Cloud_Control.ino` at a time on the ESP32-CAM. Each one owns `setup()`/`loop()` and the camera pins independently — they are not meant to run together.

> [!NOTE]
> **Board update:** The camera board used in this project is the **AI-Thinker ESP32-CAM** module (not ESP-EYE as in the previous revision of this guide). If you are following an older printout, re-check every pin table below — the GPIO map is different between the two boards.

---

## 2. Bill of Materials

| Component | Purpose |
|---|---|
| ESP32-CAM — **AI-Thinker** module (set in `board_config.h` as `CAMERA_MODEL_AI_THINKER`) | Camera + connectivity + main processing |
| Arduino **Nano** | Runs the ultrasonic sensor + buzzer |
| HC-SR04 Ultrasonic distance sensor | Detects nearby obstacles |
| Buzzer | Immediate audio alert on obstacle detection |
| I2S speaker/DAC module (e.g. MAX98357A) | Future audio output (TTS) |
| Power bank / 18650 Battery+ charging module | Powers the circuit |
| USB —JUMPER | AI-Thinker boards have **no onboard USB port** — this is required to flash and monitor the board |

---

## 3. Wiring

### 3.1 Ultrasonic Sensor + Buzzer → Arduino Nano

Per `EOS_Obstacle_Detection.ino`:

| Arduino Nano Pin | Connects to |
|---|---|
| Pin 2 | `Trig` on HC-SR04 |
| Pin 3 | `Echo` on HC-SR04 |
| Pin 4 | Buzzer (+) |
| GND | Common ground |
| 5V | HC-SR04 VCC |

### 3.2 Serial Link Between Arduino Nano and ESP32-CAM

| Arduino Nano | ESP32-CAM (AI-Thinker) |
|---|---|
| TX | U0R (GPIO3 / RX) |
| GND | GND (ground must be shared between both boards) |

> [!CAUTION]
> **Voltage mismatch — extra explanation:** The Arduino Nano runs its logic at **5V**, while the ESP32's UART pins are only safe up to **3.3V**. Wiring the Nano's TX pin directly into the ESP32's RX pin can damage the ESP32 over time. Use a simple **voltage divider** (e.g. 1kΩ + 2kΩ resistors) or a proper logic-level shifter between Nano TX and ESP32 RX. This wasn't an issue before because the previous revision of this guide didn't specify the helper board precisely — now that it's confirmed as a Nano, this step is required.

> [!IMPORTANT]
> **Current integration gap:** `EOS_Obstacle_Detection.ino` sends the string `"SNAP"` over Serial when an obstacle is detected within 30 cm. However, `EOS_Object_Detection.ino` currently only listens for `"CAPTURE"` and `"STATUS"` commands. To make the two boards actually talk to each other, add an `else if (cmd == "SNAP")` branch in `loop()` that triggers the same path as `CAPTURE`.

### 3.3 AI-Thinker ESP32-CAM — Pin Map

From `camera_pins.h`, model `CAMERA_MODEL_AI_THINKER` (also matches the pin `#define`s inside `EOS_Cloud_Control.ino`):

| Function | GPIO |
|---|---|
| PWDN | 32 |
| RESET | Unused (-1) |
| XCLK | 0 |
| SIOD (SDA) | 26 |
| SIOC (SCL) | 27 |
| Y9 – Y2 (Data) | 35, 34, 39, 36, 21, 19, 18, 5 |
| VSYNC | 25 |
| HREF | 23 |
| PCLK | 22 |
| Flash LED (onboard) | 4 |

> [!NOTE]
> Unlike ESP-EYE, the AI-Thinker board **does** use `PWDN` (GPIO 32) — don't leave it unconnected/undefined in code, or the camera may fail to power up reliably.

### 3.4 I2S Pin Check (Good News on This Board)

The firmware's I2S configuration (`EOS_Object_Detection.ino`, `Config` namespace) uses:

| I2S Signal | Assigned GPIO |
|---|---|
| BCLK | 14 |
| WS | 13 |
| DOUT | 12 |

**Unlike the ESP-EYE board**, GPIOs 12, 13, and 14 are **not** used by the camera on AI-Thinker (compare against the table in [3.3](#33-ai-thinker-esp32-cam--pin-map)) — so there's no camera/I2S pin conflict on this board. No firmware change needed here.

> [!CAUTION]
> **New caution specific to AI-Thinker — extra explanation:** GPIO12 is a **strapping pin** (MTDI) that affects flash voltage selection at boot. If it's pulled HIGH during boot, the board can fail to start correctly. It is currently used as `I2S_DOUT_PIN`. In practice this is usually safe (I2S doesn't drive DOUT until after boot), but if you ever see boot failures after wiring the I2S module, disconnect DOUT first to confirm it's the cause, and consider moving it to a non-strapping pin (GPIO15 is also a strapping pin — avoid; GPIO2 is free-ish but also affects flashing mode — test before committing to final wiring).

### 3.5 I2S Speaker Module → ESP32-CAM

| I2S Module Pin | ESP32-CAM (AI-Thinker) |
|---|---|
| BCLK | GPIO 14 |
| WS / LRC | GPIO 13 |
| DIN (on module) ← DOUT (on ESP32) | GPIO 12 (see caution above) |
| GND | GND |
| VCC | 3.3V or 5V (depending on module) |

### 3.6 New: `EOS_Cloud_Control.ino` — Cloud/Dashboard Mode

This is a **new, third firmware variant** for the ESP32-CAM. It does not talk to your own AI server at all — instead, it talks to a **Google Apps Script web app** acting as a lightweight cloud backend/dashboard. Here's exactly what it does and why each part exists:

| Step | What Happens |
|---|---|
| 1. Boot | Connects to Wi-Fi, initializes the camera (auto-adjusts frame size/JPEG quality based on PSRAM presence), then does one immediate control poll. |
| 2. Polling loop | Every `CONTROL_POLL_MS` (**5000 ms**, chosen to stay friendly with Google's free-tier Apps Script quota), it sends an HTTP GET to `APPS_SCRIPT_URL` with `?action=get&key=<ESP32_KEY>`. |
| 3. Reading the response | The response JSON is scanned (via simple string search, not a full JSON library) for two boolean fields: `flashEnabled` and `captureNow`. |
| 4. Remote flash control | If `flashEnabled` is `true`, the onboard flash LED (**GPIO4**) is switched on briefly during the next capture. |
| 5. Remote capture trigger | If `captureNow` is `true`, the board captures a frame, Base64-encodes it (via `mbedtls_base64_encode`), and POSTs it to `APPS_SCRIPT_URL?action=upload&key=<ESP32_KEY>&reason=manual`. The Apps Script backend is expected to decode it, store it (e.g. in Google Drive), and return a JSON response containing `imageUrl`. |
| 6. Auto Wi-Fi recovery | `loop()` checks `WiFi.status()` every cycle and reconnects automatically if the connection drops. |

**Configuration you must set before flashing this sketch:**

- `WIFI_SSID` / `WIFI_PASSWORD` — same as the other sketches.
- `APPS_SCRIPT_URL` — the deployed URL of your Google Apps Script web app (ends in `/exec`).
- `ESP32_KEY` — a shared secret string that **must exactly match** the key checked inside your Apps Script `Code.gs`. This is the only authentication mechanism protecting the upload/control endpoints, so treat it like a password.

> [!CAUTION]
> **Security note — extra explanation:** This sketch uses `WiFiClientSecure` with `client.setInsecure()`, which **skips TLS certificate validation**. This is a common shortcut for talking to Google Apps Script from ESP32 (avoids managing root certificates), but it means the connection is encrypted, not authenticated — a man-in-the-middle could still intercept traffic. It's an acceptable trade-off for a hobby/graduation-project dashboard, but avoid sending anything more sensitive than `ESP32_KEY` over this link, and don't reuse `ESP32_KEY` anywhere security-critical.

> [!NOTE]
> This sketch has **no local REST server** (`/status`, `/memory`, etc.) like `EOS_Object_Detection.ino` does — its "REST server" is your own Apps Script deployment, not the ESP32 itself. If you need the local diagnostics endpoints, that's another reason to keep `EOS_Object_Detection.ino` as the primary firmware and treat `EOS_Cloud_Control.ino` as an alternate mode.

---

## 4. Boot-Up Steps (From Scratch)

### a) Arduino IDE — General Setup
- Install the ESP32 board package via Boards Manager.
- Install the **ArduinoJson** library via Library Manager (required by `EOS_Object_Detection.ino`; **not** required by `EOS_Cloud_Control.ino`, which parses JSON manually).
- Confirm `CAMERA_MODEL_AI_THINKER` is selected in `board_config.h`.

### b) Flash the Helper Board (Arduino Nano)
- Open `EOS_Obstacle_Detection.ino`.
- Wire it per [Section 3.1](#31-ultrasonic-sensor--buzzer--arduino-nano), and add the voltage divider from [Section 3.2](#32-serial-link-between-arduino-nano-and-esp32-cam).
- Flash it (Serial Baud Rate = 9600). On Nano, select the correct **Old Bootloader** setting in Tools if upload fails on a clone board.

### c) Flash the ESP32-CAM — choose ONE variant
- **Production / final device:** `EOS_Object_Detection.ino` (Wi-Fi + your own AI server + I2S + local REST server).
- **Camera diagnostic only:** `EOS_Stream.ino` (live browser preview).
- **Cloud dashboard mode:** `EOS_Cloud_Control.ino` (Google Apps Script backend, remote flash/capture control).

For whichever you choose:
- Update the Wi-Fi credentials.
- AI-Thinker boards need the **IO0 → GND** jumper/button held during upload, then released after flashing, since there's no auto-reset circuit like ESP-EYE.
- Select a Partition Scheme with ≥ 3MB of APP space.
- Flash the sketch, then open Serial Monitor at 115200 baud.

### d) Verify Connectivity (`EOS_Object_Detection.ino` only)

| Endpoint | Purpose |
|---|---|
| `http://<ESP32_IP>/status` | System status + last AI result |
| `http://<ESP32_IP>/memory` | Memory diagnostics |
| `http://<ESP32_IP>/capture` | Triggers an immediate capture + AI inference |
| `http://<ESP32_IP>/health` | Quick liveness check |

For `EOS_Cloud_Control.ino`, "verifying connectivity" instead means checking your Google Apps Script dashboard/sheet for the polled status and confirming an uploaded `imageUrl` appears after triggering `captureNow`.

---

## 5. File Reference

| File | Purpose |
|---|---|
| `EOS ESP32CAM/EOS_Object_Detection.ino` | **Main production firmware** — camera + Wi-Fi + AI server upload + I2S + local REST server. |
| `EOS ESP32CAM/EOS_Cloud_Control.ino` | **New.** Cloud/dashboard-controlled variant — polls a Google Apps Script backend for remote flash/capture commands and uploads Base64-encoded images to it. See [Section 3.6](#36-new-eos_cloud_controlino--cloud-dashboard-mode). |
| `EOS_Stream.ino` | Live camera stream example, based on Espressif's official `CameraWebServer`. Diagnostic use only. |
| `app_httpd.cpp` | The internal HTTP server started by `EOS_Stream.ino`. |
| `camera_index.h` | The web UI (HTML/JS) served by `EOS_Stream.ino`. |
| `EOS ARDUIONO/EOS_Obstacle_Detection.ino` | Runs on the Arduino Nano (ultrasonic + buzzer); sends `"SNAP"` over Serial when an obstacle is detected. |
| `board_config.h` | Where the camera board model is selected (now `CAMERA_MODEL_AI_THINKER`). |
| `camera_pins.h` | Pin definitions for every supported camera board model. |
| `config.yml` | Optional CI/build configuration file. |

---

## 6. Which Sketch Should I Actually Use?

- **For the actual glasses (final goal):** flash only `EOS_Object_Detection.ino` on the ESP32-CAM, together with `EOS_Obstacle_Detection.ino` on the Arduino Nano.
- **To just confirm the camera works during assembly (diagnostic step):** temporarily flash `EOS_Stream.ino`, then re-flash `EOS_Object_Detection.ino` afterward.
- **If you want remote monitoring/control through a simple web dashboard without hosting your own AI server:** flash `EOS_Cloud_Control.ino` instead — it hands flash and capture control to a Google Apps Script web app. This is a separate deployment mode, not a drop-in replacement for the AI-inference pipeline in `EOS_Object_Detection.ino`.

---

## 7. Pre-Flight Checklist

**Common to all variants**
- [ ] Camera model in `board_config.h` is `CAMERA_MODEL_AI_THINKER`
- [ ] IO0 → GND held during flashing (AI-Thinker has no auto-reset)
- [ ] `WIFI_SSID` / `WIFI_PASSWORD` are correct
- [ ] Ground (GND) is shared across all devices (Arduino Nano + ESP32 + sensors)
- [ ] Voltage divider/level shifter added between Nano TX and ESP32 RX ([Section 3.2](#32-serial-link-between-arduino-nano-and-esp32-cam))

**`EOS_Object_Detection.ino` only**
- [ ] `AI_SERVER_URL` points to a running server listening on `/predict`
- [ ] ArduinoJson library is installed in the Arduino IDE
- [ ] If enabling `SNAP` integration, `loop()` has been updated accordingly
- [ ] I2S DOUT boot behavior tested once (GPIO12 strapping pin caution, [Section 3.4](#34-i2s-pin-check-good-news-on-this-board))

**`EOS_Cloud_Control.ino` only**
- [ ] `APPS_SCRIPT_URL` set to your deployed Apps Script `/exec` URL
- [ ] `ESP32_KEY` matches the key checked in `Code.gs` exactly
- [ ] Comfortable with the `setInsecure()` TLS trade-off ([Section 3.6](#36-new-eos_cloud_controlino--cloud-dashboard-mode))
