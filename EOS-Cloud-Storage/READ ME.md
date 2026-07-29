# EOS — Cloud Dashboard Module

**Google Apps Script + Google Drive + Google Sheets backend for remote camera control**

This document covers the cloud/software side of the `EOS_Cloud_Control.ino` firmware variant: a serverless backend built entirely on free Google services, plus a web dashboard for remote flash control and photo capture.

> **Repo location:** these files live in `Graduated-Project/EOS-Cloud-Storage/` — this README is meant to sit in that same folder as `EOS-Cloud-Storage/README.md`.

> Setup screenshots in this guide are placed in [`docs/images/`](./docs/images) — see [Section 11](#11-adding-the-screenshots-to-this-repo) for how to make them render on GitHub.

---

## 1. Overview

This module lets you trigger the ESP32-CAM to take a photo and toggle its flash **from any browser**, with no server of your own to host or maintain. It replaces a traditional backend with three free Google services working together:

| Service | Role |
|---|---|
| **Google Apps Script** | Acts as the HTTPS API the ESP32-CAM and the dashboard both talk to |
| **Google Sheets** | Stores current device state (`flashEnabled`, `captureNow`, latest image URL) and an activity log |
| **Google Drive** | Stores the actual captured JPEG files |

The dashboard is a PIN-protected single page, and the ESP32-CAM authenticates with a shared secret key. No paid infrastructure is required.

---

## 2. Architecture

```
 ┌─────────────┐   polls every 5s    ┌──────────────────┐
 │  ESP32-CAM  │ ───────────────────▶│  Apps Script      │
 │ (AI-Thinker)│  GET ?action=get     │  Web App (Code.gs)│
 │             │◀─────────────────── │                    │
 │             │   {flashEnabled,     │  doGet / doPost    │
 │             │    captureNow}       │                    │
 │             │                      └─────────┬──────────┘
 │             │  POST ?action=upload           │
 │             │  (Base64 JPEG)                  │ reads/writes
 │             │ ───────────────────────────────▶│
 └─────────────┘                                 ▼
                                        ┌──────────────────┐
                                        │  Google Sheet     │
                                        │  (state + logs)   │
                                        └─────────┬──────────┘
                                                  │
                                                  ▼
                                        ┌──────────────────┐
                                        │  Google Drive     │
                                        │  (image files)     │
                                        └──────────────────┘

 ┌─────────────┐   PIN + google.script.run   ┌────────────┐
 │  Dashboard  │ ────────────────────────────▶│  Code.gs   │
 │ (Index.html)│  verifyPin / getStateForHtml │  (server-  │
 │             │  / setStateFromHtml           │   side)    │
 └─────────────┘                              └────────────┘
```

The ESP32-CAM and the dashboard never talk to each other directly — both go through the same Apps Script deployment, which is the single source of truth for device state.

---

## 3. Files in This Module

| Repo file name | Must be renamed to (inside the Apps Script project) | Purpose |
|---|---|---|
| `Environment Cloud.gs` | **`Code.gs`** | The Apps Script backend: HTTP API (`doGet`/`doPost`) + dashboard-facing functions |
| `EOS App Storage.html` | **`Index.html`** | The dashboard UI served to the browser |
| `EOS_Cloud_Control.ino` (in the `AI SERVER` or `Hardware` folder — the ESP32-CAM firmware) | — | The firmware that polls this backend and uploads photos |

> [!IMPORTANT]
> Apps Script resolves included files **by exact name**, and `Code.gs` calls `HtmlService.createHtmlOutputFromFile("Index")`. When you create these two files inside the Apps Script editor, they must be named exactly `Code` and `Index` (Apps Script hides the `.gs`/`.html` extension in the file list) — not `Environment_Cloud` or `EOS_App_Storage`. If the names don't match, `doGet()` will throw an error instead of serving the dashboard.

---

## 4. Backend Setup (Google Apps Script) — Step by Step

**Step 1 — Create the Google Sheet.**
Create a new Google Sheet (e.g. named `WiFi Camera V1`). It will store camera settings, capture requests, and the latest image info. Open **Extensions → Apps Script**.

![Creating the Google Sheet and opening Apps Script]<img width="1104" height="478" alt="01-create-google-sheet-apps-script" src="https://github.com/user-attachments/assets/6d2ab25e-fc4b-4d62-b7fd-d73f5514e25f" />

**Image 1:** Creating the Google Sheet and opening Extensions → Apps Script.

**Step 2 — Add the two project files.**
Inside the Apps Script editor, use the **+** next to "Files" to add a **Script** file and an **HTML** file.

![Adding a Script file and an HTML file in the Apps Script editor]<img width="1096" height="457" alt="02-add-code-gs-and-index-html" src="https://github.com/user-attachments/assets/681c7906-db39-4641-9cd7-b12c43f399c5" />

**Image 2:** Adding a new Script file (`Code`) and a new HTML file (`Index`) inside the Apps Script editor.

- Name the script file `Code` → paste in the contents of `Environment Cloud.gs`.
- Name the HTML file `Index` → paste in the contents of `EOS App Storage.html`.

**Step 3 — Fill in your own settings at the top of `Code.gs`.**

![Editing the USER SETTINGS block at the top of Code.gs]<img width="1114" height="463" alt="03-code-gs-user-settings" src="https://github.com/user-attachments/assets/e9fa855a-0240-4ae7-88cd-f7cca3508bef" />

**Image 3:** Filling in `SPREADSHEET_ID`, `DRIVE_FOLDER_ID`, `ESP32_KEY`, and `DASHBOARD_PIN` at the top of `Code.gs`.

```js
const SPREADSHEET_ID = "PASTE_YOUR_GOOGLE_SHEET_ID";
const DRIVE_FOLDER_ID = "PASTE_YOUR_DRIVE_FOLDER_ID";
const ESP32_KEY = "esp32_cam_key_123";   // must match the Arduino sketch exactly
const DASHBOARD_PIN = "SET PIN";          // change this to a real PIN
```

**Step 4 — Deploy as a Web App.**
`Deploy → New deployment → Web app`, with **Execute as: Me** and **Who has access: Anyone**. Google requires "Anyone" here because the ESP32-CAM cannot sign in to a Google account — it only reaches the endpoint over plain HTTPS. Custom security still comes from `ESP32_KEY` and `DASHBOARD_PIN`, not from Google's access control.

Copy the generated **Web app URL** — this is your `APPS_SCRIPT_URL`.

![Copying the deployed Web App URL]<img width="1131" height="519" alt="04-index-html-login-wiring" src="https://github.com/user-attachments/assets/9c812616-b153-4427-97c2-3ab7098f8a96" />

**Image 4:** Copying the deployed Web App URL after clicking Deploy — this becomes `APPS_SCRIPT_URL`.

> [!NOTE]
> The first deployment may trigger a Google "unverified app" warning, since the script runs under your personal account and hasn't been submitted for Google review. This is expected — click **Advanced → Go to (project name) (unsafe) → Allow**. You only need to do this once, unless you redeploy with new permission scopes.

**Step 5 — Confirm the backend sheets.**
On first request, `ensureSheets_()` automatically creates two tabs at the bottom of your spreadsheet if they don't already exist: `state` (current device state, single row) and `logs` (append-only history).

![The auto-created "state" and "logs" tabs with live data]<img width="1124" height="633" alt="05-deploy-copy-web-app-url" src="https://github.com/user-attachments/assets/73bf3cd3-686f-4f0b-9685-bc37c99ea352" />

**Image 5:** The auto-created `state` and `logs` tabs at the bottom of the spreadsheet, showing live data after the first requests.

---

## 5. Data Model

### `state` sheet (single row, columns A–F)

| Column | Field | Type | Meaning |
|---|---|---|---|
| A | `flashEnabled` | boolean | Whether the flash should fire on next capture |
| B | `captureNow` | boolean | Set to `true` by the dashboard; the ESP32 clears it after uploading |
| C | `latestImageUrl` | string | Public view link to the most recent Drive image |
| D | `lastCaptureAt` | datetime | Timestamp of the last successful capture |
| E | `lastReason` | string | Free-text tag for why the capture happened (e.g. `"manual"`) |
| F | `updatedAt` | datetime | Timestamp of the last state write, from any source |

### `logs` sheet (append-only)

| Column | Field |
|---|---|
| A | `timestamp` |
| B | `source` (`"dashboard"`, `"upload"`, etc.) |
| C | `flashEnabled` |
| D | `captureNow` |
| E | `latestImageUrl` |
| F | `reason` |

---

## 6. HTTP API Reference (used by the ESP32-CAM)

### `GET ?action=get&key=<ESP32_KEY>`
Returns the current control state. Called by the firmware every 5 seconds.

```json
{
  "ok": true,
  "data": {
    "flashEnabled": false,
    "captureNow": true,
    "latestImageUrl": "https://drive.google.com/uc?export=view&id=...",
    "lastCaptureAt": "2026-07-29 10:15:03",
    "lastReason": "manual",
    "updatedAt": "2026-07-29 10:15:03"
  }
}
```

If `key` doesn't match `ESP32_KEY`: `{"ok": false, "error": "unauthorized"}`.

### `POST ?action=upload&key=<ESP32_KEY>&reason=<text>`
Body: raw Base64-encoded JPEG (plain text, not JSON). Called by the firmware after a capture.

- Decodes the Base64 payload, saves it to `DRIVE_FOLDER_ID` as `esp32cam_<reason>_<timestamp>.jpg`.
- Sets the file's sharing permission to **Anyone with the link — Viewer**.
- Writes the new `latestImageUrl` into the `state` sheet and clears `captureNow`.
- Appends a row to `logs`.

```json
{
  "ok": true,
  "imageUrl": "https://drive.google.com/uc?export=view&id=...",
  "data": { "...": "updated state object" }
}
```

Any other `action` value, or a bad key, returns `{"ok": false, "error": "..."}`.

### Dashboard-only functions (called via `google.script.run`, not raw HTTP)
These are meant to be invoked from `Index.html` inside the Apps Script sandbox, not from the ESP32:

| Function | Purpose |
|---|---|
| `verifyPin(pin)` | Returns `true`/`false` — checks the entered PIN against `DASHBOARD_PIN` |
| `getStateForHtml(pin)` | Returns the current state object, but throws if the PIN is wrong |
| `setStateFromHtml(flashEnabled, captureNow, pin)` | Writes new flash/capture flags, PIN-gated |

---

## 7. ⚠️ Current Integration Gap — Dashboard Is Not Yet Wired to the Backend

> [!CAUTION]
> **This is the most important note in this document.** As currently written, `EOS App Storage.html` (→ `Index.html`) is a **self-contained visual mockup**. It does **not** call `google.script.run` anywhere:
> - The PIN check is done entirely in client-side JavaScript against a hardcoded `const PIN='1234'` — it never calls the real `verifyPin()` function in `Code.gs`, and `DASHBOARD_PIN` from your backend is never checked.
> - The **Capture** button's `capture()` function just runs a `setTimeout(...)` for 2.8 seconds and then fakes a "success" state (`shots++`, a canned toast message) — it never calls `setStateFromHtml()` or hits the Apps Script endpoint at all.
> - The **Flash** toggle only updates the UI locally; it never persists to the `state` sheet.
>
> In its current form, the dashboard will look and feel functional but **will not actually trigger the ESP32-CAM** to do anything.

**For comparison, here is what correct wiring looks like** — this is the reference tutorial's `login()` function, which calls `google.script.run` with success/failure handlers before invoking the real `verifyPin(pin)`:

![Reference implementation: login() correctly calling google.script.run.verifyPin(pin)]<img width="1017" height="588" alt="07-state-and-logs-sheets" src="https://github.com/user-attachments/assets/d8f744db-4e95-4669-88f2-f77586465046" />

**Image 6:** Reference implementation — `login()` correctly calling `google.script.run...verifyPin(pin)` instead of checking a hardcoded PIN.

The same pattern needs to be applied to the Capture and Flash controls in `EOS App Storage.html`, e.g.:

```js
google.script.run
  .withSuccessHandler(onCaptureConfirmed)
  .withFailureHandler(onError)
  .setStateFromHtml(flashEnabled, true, enteredPin);
```

and the login screen's PIN check needs to call `verifyPin(pin)` through `google.script.run` instead of comparing against the hardcoded `PIN` constant. If you'd like, I can rewrite `EOS App Storage.html` to wire these calls in properly — just ask.

---

## 8. ESP32-CAM Programming Method (MB Board + Jumper)

This backend is designed to pair with `EOS_Cloud_Control.ino`, flashed using the **ESP32-CAM-MB programmer board** (no separate FTDI adapter needed):

1. Seat the ESP32-CAM firmly onto the MB board, matching the pin headers.
2. **Install the jumper wire between GPIO0 and GND.** This is the exact method used in this build — the jumper forces the board into boot/download mode and **must stay connected for the entire upload process.** Do not remove it until step 6 below.
3. Connect a micro-USB cable from the MB board to your computer (this also powers the board).
4. In Arduino IDE: select board **"AI Thinker ESP32-CAM"**, pick the COM port that appears (MB boards typically use a CH340 USB-serial chip), and use the **Huge APP** partition scheme.
5. Set `WIFI_SSID`, `WIFI_PASSWORD`, `APPS_SCRIPT_URL` (your `/exec` link), and `ESP32_KEY` (must match `Code.gs` exactly) inside the sketch, then upload.

   ![Arduino sketch configuration block: WiFi credentials, Apps Script URL, ESP32_KEY]<img width="1120" height="343" alt="06-arduino-sketch-config" src="https://github.com/user-attachments/assets/e373c9a9-19c0-4ed8-bb94-334b2c96dc98" />

   **Image 7:** Arduino sketch configuration block — `WIFI_SSID`, `WIFI_PASSWORD`, `APPS_SCRIPT_URL`, and `ESP32_KEY`.

6. After a successful upload, **remove the GPIO0–GND jumper** (this is the only point in the whole process where it should come off) and press reset (or power-cycle) to boot normally.
7. Open Serial Monitor at 115200 baud to confirm Wi-Fi connects and polling starts.

> [!NOTE]
> **Jumper reminder:** if the board fails to be detected for upload, double-check the GPIO0–GND jumper is still seated before retrying — it's the single most common reason an AI-Thinker board doesn't enter upload mode on the MB board. If the board boots straight into your program instead of showing up as a COM port ready for flashing, the jumper likely isn't making contact.

---

## 9. Security Notes

- **`ESP32_KEY`** is the only thing preventing anyone with your `/exec` URL from remotely triggering captures or reading state. Treat it as a password — don't commit a real production key to a public repo.
- **`DASHBOARD_PIN`** protects the dashboard, but see [Section 7](#7-️-current-integration-gap--dashboard-is-not-yet-wired-to-the-backend) — until the dashboard is wired to `verifyPin()`, the PIN check is cosmetic only (hardcoded `'1234'` client-side).
- Uploaded images are set to **"Anyone with the link — Viewer"** on Google Drive. Anyone who obtains an image URL can view that specific photo without authentication. This is required for the dashboard's `<img>` preview to work without OAuth, but it means links should be treated as semi-public.
- The firmware side (`EOS_Cloud_Control.ino`) uses `WiFiClientSecure` with `client.setInsecure()`, skipping TLS certificate validation. Acceptable for a graduation project, but worth mentioning if this is evaluated or extended for production use.

---

## 10. Setup Checklist

- [ ] Google Sheet created, `SPREADSHEET_ID` copied into `Code.gs`
- [ ] Drive folder created, `DRIVE_FOLDER_ID` copied into `Code.gs`
- [ ] `ESP32_KEY` and `DASHBOARD_PIN` changed from their placeholder values
- [ ] Apps Script files named exactly `Code` and `Index` (see [Section 3](#3-files-in-this-module))
- [ ] Deployed as Web App (**Execute as: Me**, **Who has access: Anyone**)
- [ ] Google "unverified app" warning accepted once (Advanced → Go to project → Allow)
- [ ] `/exec` URL copied into `EOS_Cloud_Control.ino` as `APPS_SCRIPT_URL`
- [ ] `state` and `logs` sheets confirmed auto-created after first request
- [ ] Aware that the dashboard UI is currently not calling the backend ([Section 7](#7-️-current-integration-gap--dashboard-is-not-yet-wired-to-the-backend)) — decide whether to wire it up before demo day
- [ ] GPIO0–GND jumper removed after flashing, board reset to run normally

---

