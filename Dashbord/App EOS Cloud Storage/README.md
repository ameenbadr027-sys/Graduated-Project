# 🔐 Secure PIN Authentication

Before accessing the EOS Cloud Dashboard, users must authenticate through a secure PIN-based login system. This security layer prevents unauthorized access to the ESP32-CAM, cloud storage services, and device controls.

Only authenticated users can operate the camera, capture images, manage cloud synchronization, or modify system settings.

<p align="center">
   <img width="727" height="840" alt="Screenshot 2026-07-29 143104" src="https://github.com/user-attachments/assets/41ec64a3-f324-41c2-80be-4287049fcce1" />

</p>

### Security Features

* Secure PIN verification
* Protected access to camera controls
* Restricted cloud storage operations
* Secure session management
* Unauthorized access prevention

---

# 📸 EOS Cloud Dashboard

The EOS Cloud Dashboard is a secure web-based control center developed for the ESP32-CAM (OV2640) used in the EOS Smart Assistive Glasses project. It provides an intuitive interface for remotely controlling the camera, capturing images, managing device settings, and synchronizing captured data with Google Drive and Google Sheets.

<p align="center">
  <img width="970" height="871" alt="Screenshot 2026-07-29 143146" src="https://github.com/user-attachments/assets/ddb69f8f-a6c8-4d91-b668-95f66a4c516a" />

</p>

## Overview

The dashboard serves as the central communication hub between the ESP32-CAM and the cloud infrastructure. Through a single interface, users can securely manage the vision module while monitoring device activity in real time.

The interface combines camera control, image acquisition, cloud synchronization, and system monitoring into one integrated platform.

---

## Camera Control

The dashboard communicates directly with the ESP32-CAM through HTTP requests, allowing users to remotely configure and operate the OV2640 camera.

Supported capabilities include:

* Capture high-quality images
* Flash LED control (GPIO 4)
* Camera preview
* HDR Mode
* Automatic cloud synchronization
* Device monitoring
* System configuration

These controls allow the camera to be adjusted without modifying or reflashing the firmware.

---

## Image Capture

When the **Capture Photo** button is pressed, the dashboard sends a capture request to the ESP32-CAM.

The camera captures a new image and immediately returns it to the dashboard, where it is displayed for preview. This enables users to verify image quality before the image is processed by the AI object detection system.

---

## Automatic Google Drive Storage

If **Auto Save** is enabled, every captured image is automatically uploaded to Google Drive.

This provides secure cloud storage for captured images while eliminating the need for manual uploads.

Cloud storage offers several advantages:

* Permanent image archiving
* Remote accessibility
* AI dataset generation
* Easy sharing through Google Drive
* Reliable backup of captured images

---

## Google Sheets Logging

Every successful image capture is automatically recorded in a connected Google Sheets document.

Each new record stores metadata associated with the captured image, creating a complete capture history.

The recorded information includes:

* Capture Date
* Capture Time
* Upload Timestamp
* Google Drive File Link
* Upload Status
* Image Identifier
* Device Information (when available)

This logging mechanism provides a structured database that can later be used for tracking, debugging, analytics, or AI dataset management.

---

## Live Dashboard Statistics

The dashboard continuously updates important system statistics in real time, including:

* Total Photos Captured
* Last Capture Time
* Flash Status
* Number of Images Uploaded to Google Drive
* Device Connection Status
* Current System Activity

These statistics help monitor the overall health and performance of the ESP32-CAM throughout operation.

---

## Activity Log

Every operation performed by the dashboard is recorded inside the Activity Log.

Typical events include:

* User authentication
* Camera initialization
* Image capture
* Flash activation
* Google Drive upload
* Google Sheets logging
* Device status updates
* System notifications
* Error messages

The activity log simplifies debugging while providing a complete history of system events.

---

## System Workflow

The dashboard follows a fully automated workflow that connects the ESP32-CAM with Google's cloud services.

```text
User Authentication (PIN)
            │
            ▼
EOS Cloud Dashboard
            │
            ▼
Capture Photo
            │
            ▼
ESP32-CAM (OV2640)
            │
            ▼
Return Captured Image
            │
            ├────────► Display Image Preview
            │
            ├────────► Upload Image to Google Drive
            │
            ├────────► Generate Shareable Drive Link
            │
            └────────► Save Metadata to Google Sheets
                            │
                            ▼
               Date • Time • Timestamp • Drive URL
```

---

## Key Features

* Secure PIN Authentication
* Remote ESP32-CAM Control
* OV2640 Camera Integration
* High-Resolution Image Capture
* Flash LED Control
* Live Image Preview
* Automatic Google Drive Upload
* Automatic Google Sheets Logging
* Real-Time Dashboard Statistics
* Activity Monitoring
* Cloud-Based Image Archive
* Historical Capture Records
* Dataset Collection for AI Applications
* Centralized Device Management

---

The EOS Cloud Dashboard is more than a camera interface—it is a complete cloud-enabled management platform that securely controls the ESP32-CAM, automates image storage, maintains detailed capture records, and provides a reliable foundation for the EOS AI vision pipeline used in smart assistive glasses.
