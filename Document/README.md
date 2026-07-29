<div align="center">



# EOS

### AI-Powered Smart Assistive Glasses for the Visually Impaired

<p align="center">

<img src="https://img.shields.io/badge/Embedded-ESP32--CAM-red?style=for-the-badge"/>

<img src="https://img.shields.io/badge/Arduino-Nano-blue?style=for-the-badge"/>

<img src="https://img.shields.io/badge/Computer%20Vision-Object%20Detection-success?style=for-the-badge"/>

<img src="https://img.shields.io/badge/Cloud-Google%20Drive-4285F4?style=for-the-badge"/>

<img src="https://img.shields.io/badge/Database-Google%20Sheets-34A853?style=for-the-badge"/>

<img src="https://img.shields.io/badge/AI-Powered-orange?style=for-the-badge"/>

<img src="https://img.shields.io/badge/Status-Active-success?style=for-the-badge"/>

</p>

An intelligent wearable system that combines **Artificial Intelligence**, **Computer Vision**, **Embedded Systems**, and **Cloud Computing** to provide real-time assistance for visually impaired individuals.

</div>

---

# 📖 Table of Contents

- Overview
- Motivation
- Objectives
- System Features
- System Architecture
- Hardware Components
- Software Components
- Dashboard
- Cloud Storage
- Artificial Intelligence
- Object Detection Workflow
- Repository Structure
- Technologies
- Installation
- Configuration
- Running the Project
- Contributors
- License

---

# 📚 Project Overview

EOS (Eye-Oriented Smart Assistant) is an intelligent assistive system designed to improve the independence of visually impaired individuals by combining embedded hardware with artificial intelligence.

The project captures images using an ESP32-CAM equipped with the OV2640 camera sensor. Captured images are securely uploaded to Google Drive while their metadata—including timestamps and storage links—is automatically recorded in Google Sheets. An AI processing pipeline analyzes the images to identify surrounding objects and generate meaningful information that can later be converted into spoken feedback.

EOS is built as a modular platform where each subsystem performs a dedicated task, making the project scalable, maintainable, and easy to extend with additional AI capabilities or hardware modules.

---

# 💡 Motivation

Millions of visually impaired individuals rely on traditional mobility aids that provide limited environmental awareness. While these tools help users avoid obstacles, they cannot describe surrounding objects or provide contextual information.

EOS addresses this challenge by integrating computer vision, cloud services, and embedded systems into a single wearable platform capable of recognizing objects and assisting users in real time.

The primary goal is not only to detect obstacles but also to enhance environmental understanding through intelligent image analysis.

---

# 🎯 Objectives

The EOS project has been developed with the following objectives:

- Assist visually impaired individuals through AI-powered vision.
- Capture high-quality images using the ESP32-CAM.
- Build a secure cloud-based image archive.
- Automatically log image metadata in Google Sheets.
- Provide a modern web interface for camera management.
- Enable remote monitoring of the embedded system.
- Support future AI models without hardware redesign.
- Maintain a modular and scalable software architecture.

---

# ✨ System Features

## 👓 Smart Vision

- AI-powered object recognition
- Environmental awareness
- Future voice guidance support
- Intelligent image processing

---

## 📷 Camera Module

- ESP32-CAM
- OV2640 Camera Sensor
- High-resolution image capture
- Flash LED support
- Adjustable camera parameters

---

## 🌐 Web Dashboard

- Secure PIN authentication
- Camera configuration
- Live image preview
- Flash control
- Device monitoring
- Cloud synchronization
- Activity logging

---

## ☁ Cloud Integration

- Automatic Google Drive uploads
- Automatic Google Sheets logging
- Secure cloud storage
- Image history management
- AI dataset generation

---

## 🤖 Artificial Intelligence

- Object Detection
- Computer Vision
- Image Analysis
- AI Server Integration
- Scalable AI pipeline

---

# 📸 System Preview

## Secure Authentication

Before accessing the system, authorized users must enter a secure PIN. This authentication layer protects the dashboard and prevents unauthorized access to the ESP32-CAM, cloud storage services, and device management functions.

<p align="center">

</p>

---

## EOS Cloud Dashboard

The EOS Cloud Dashboard acts as the central control hub for the entire vision subsystem.

From a single interface, users can:

- Configure the camera.
- Capture images remotely.
- Control the flash LED.
- Preview captured images.
- Upload images to Google Drive.
- Log metadata into Google Sheets.
- Monitor system status.
- View activity logs.

<p align="center">

</p>
---

# 🏗 System Architecture

EOS follows a modular architecture where each subsystem performs a dedicated responsibility. This separation improves maintainability, scalability, and simplifies future development.

<p align="center">

</p>

The system consists of five primary layers:

1. Hardware Layer
2. Embedded Control Layer
3. Cloud Management Layer
4. Artificial Intelligence Layer
5. User Interaction Layer

Each layer communicates with the next through lightweight interfaces, enabling reliable and efficient data flow across the entire system.

---

# ⚙ Hardware Architecture

The hardware subsystem is responsible for capturing visual information, controlling peripheral devices, and delivering audio feedback.

## Main Components

| Component | Purpose |
|------------|---------|
| ESP32-CAM | Image acquisition and Wi-Fi communication |
| OV2640 Camera | Captures high-resolution images |
| Arduino Nano | Main embedded controller |
| DFPlayer Mini | Audio playback module |
| Speaker | Voice feedback |
| Push Buttons | User interaction |
| Battery | Portable power source |

---

## ESP32-CAM

The ESP32-CAM acts as the vision module of EOS.

Its responsibilities include:

- Capturing images using the OV2640 camera sensor.
- Hosting the HTTP camera server.
- Processing camera configuration requests.
- Uploading captured images.
- Streaming camera data.
- Communicating with the Cloud Dashboard.

Key Features:

- OV2640 Camera Sensor
- Wi-Fi Connectivity
- JPEG Compression
- HTTP Server
- Flash LED Support
- High Resolution Image Capture

---

## Arduino Nano

The Arduino Nano functions as the central controller responsible for coordinating system operations.

Responsibilities include:

- Communicating with external modules.
- Receiving commands.
- Managing audio playback.
- Synchronizing hardware components.
- Executing system logic.

---

## Audio Module

EOS provides spoken feedback using a DFPlayer Mini connected to a speaker.

Future AI-generated descriptions can be converted into voice messages, allowing visually impaired users to receive immediate auditory assistance.

---

# 🌐 Dashboard Architecture

The EOS Cloud Dashboard serves as the primary user interface for interacting with the ESP32-CAM.

It enables secure remote operation of the vision subsystem while providing cloud synchronization and monitoring capabilities.

<p align="center">

</p>

---

## Secure Authentication

Before accessing the dashboard, users must authenticate using a secure PIN.

<p align="center">

</p>

Authentication protects:

- Camera controls
- Cloud storage
- Device configuration
- Image capture
- Administrative operations

Only authenticated users are allowed to interact with the embedded system.

---

## Camera Control

The dashboard provides complete remote configuration of the OV2640 camera.

Supported camera parameters include:

- Frame Size
- JPEG Quality
- Brightness
- Contrast
- Saturation
- Sharpness
- White Balance
- Auto Exposure
- Auto Gain
- Lens Correction
- Horizontal Mirror
- Vertical Flip
- Special Effects

Changes are applied immediately without requiring firmware updates or device restarts.

---

## Image Capture

When the user presses the **Capture Photo** button, the dashboard sends an HTTP request to the ESP32-CAM.

The camera captures a new image and returns it to the dashboard for immediate preview.

This allows developers to verify image quality before it enters the AI processing pipeline.

---

## Flash Control

The integrated flash LED can be enabled directly from the dashboard.

This improves visibility in low-light environments and significantly increases image quality during night-time or indoor operation.

---

## Device Monitoring

The dashboard continuously monitors the ESP32-CAM and displays important runtime information.

Displayed information includes:

- Device Status
- Connection Status
- Wi-Fi Availability
- System Uptime
- Last Activity
- Camera Availability
- Stream Status

This information helps developers monitor hardware health during deployment and testing.

---

## Activity Console

Every operation performed within the dashboard is recorded in a real-time activity log.

Typical events include:

- User Login
- Camera Initialization
- Image Capture
- Flash Activation
- Google Drive Upload
- Google Sheets Logging
- Device Restart
- Configuration Changes
- Error Messages

The activity console greatly simplifies debugging while maintaining a complete operational history.

---

# ☁ Cloud Integration

One of EOS's primary features is seamless cloud synchronization.

Immediately after image capture, the system automatically archives the image and stores all related metadata.

Cloud services eliminate manual file management while creating a permanent dataset for future AI development.

---

## Google Drive Storage

Captured images are automatically uploaded to Google Drive.

Advantages include:

- Secure storage
- Remote access
- Permanent backup
- Dataset creation
- Easy sharing

Every uploaded image receives its own unique Drive URL.

---

## Google Sheets Logging

In addition to image storage, EOS automatically records capture metadata inside Google Sheets.

Each capture generates a new database record containing:

- Capture Date
- Capture Time
- Upload Timestamp
- Image Name
- Google Drive URL
- Upload Status
- Device Information

This creates a searchable history of every captured image.

---

## Automated Cloud Workflow

```text
Capture Photo
      │
      ▼
ESP32-CAM
      │
      ▼
Dashboard
      │
      ├────────► Upload Image to Google Drive
      │
      ├────────► Generate Drive URL
      │
      └────────► Store Metadata in Google Sheets
                          │
                          ▼
               Complete Cloud Archive
```

The entire process is fully automated and requires no manual intervention from the user.

---
# 🤖 Artificial Intelligence Module

Artificial Intelligence is the core component that transforms captured images into meaningful information. The AI module is responsible for analyzing visual data, recognizing surrounding objects, and generating descriptive outputs that can later be converted into spoken feedback for visually impaired users.

The AI subsystem has been designed to be modular, allowing different computer vision models to be integrated without requiring modifications to the embedded hardware.

---

## AI Responsibilities

The AI server performs several important tasks:

- Receive captured images.
- Preprocess images.
- Perform object detection.
- Generate detection results.
- Return structured information.
- Support future AI model upgrades.

---

## Image Processing Pipeline

Every image captured by the ESP32-CAM passes through a complete processing pipeline before the final result is produced.

```text
Capture Image
      │
      ▼
Image Upload
      │
      ▼
Image Preprocessing
      │
      ▼
AI Model
      │
      ▼
Object Detection
      │
      ▼
Confidence Evaluation
      │
      ▼
Generate Detection Results
      │
      ▼
Voice Feedback
```

---

# 🔍 Object Detection

Object Detection enables EOS to identify surrounding objects and provide contextual awareness.

The AI model analyzes each captured frame and determines:

- Object Class
- Confidence Score
- Bounding Box
- Object Location

Future versions of EOS may support:

- Face Recognition
- Scene Understanding
- OCR
- Currency Recognition
- Traffic Sign Recognition
- Obstacle Classification

---

# 📡 Communication Flow

EOS consists of multiple independent modules communicating together.

```text
ESP32-CAM
      │
HTTP Request
      │
      ▼
Cloud Dashboard
      │
      ▼
Google Drive
      │
      ▼
Google Sheets
      │
      ▼
AI Server
      │
      ▼
Detection Result
      │
      ▼
Audio Controller
      │
      ▼
User
```

Every module has a dedicated responsibility, making the system scalable and easier to maintain.

---

# 🔄 Complete System Workflow

The following workflow illustrates the complete lifecycle of a captured image.

```text
User

   │

   ▼

Open Dashboard

   │

   ▼

PIN Authentication

   │

   ▼

Dashboard Access

   │

   ▼

Capture Image

   │

   ▼

ESP32-CAM

   │

   ▼

Return Image Preview

   │

   ▼

Upload Image

   │

   ├────────► Google Drive

   │

   └────────► Google Sheets

                    │

                    ▼

              AI Processing

                    │

                    ▼

            Object Detection

                    │

                    ▼

             Voice Description

                    │

                    ▼

            Visually Impaired User
```

---

# 📂 Repository Structure

```text
EOS
│
├── AI SERVER
│   ├── Models
│   ├── Detection Engine
│   ├── API
│   └── Utilities
│
├── Dashboard
│   ├── HTML
│   ├── CSS
│   ├── JavaScript
│   ├── Assets
│   └── Components
│
├── EOS Cloud Storage
│   ├── Google Apps Script
│   ├── Drive Integration
│   └── Google Sheets Integration
│
├── Hardware
│   ├── ESP32-CAM
│   ├── Arduino Nano
│   ├── Wiring Diagram
│   └── Libraries
│
├── docs
│
├── images
│
├── README.md
│
└── LICENSE
```

---

# 🧩 Design Principles

EOS has been developed following modern software engineering principles.

### Modularity

Every subsystem performs an independent task, making the project easier to maintain and extend.

### Scalability

New AI models, sensors, or cloud services can be integrated without redesigning the complete system.

### Reliability

Cloud storage guarantees that captured images remain available even after device restarts.

### Maintainability

The repository is organized into independent modules with clear responsibilities.

### Extensibility

Future features can be added with minimal impact on existing components.

---

# 🔐 Security

EOS includes multiple security mechanisms to protect both hardware and cloud resources.

Current security features include:

- Secure PIN Authentication
- Protected Dashboard Access
- Restricted Camera Control
- Authorized Cloud Operations
- Controlled Image Capture

Future versions may include:

- User Accounts
- Multi-Factor Authentication
- Encrypted Communication
- Role-Based Permissions
- Secure Cloud Tokens

---
# ⚙️ Installation

Follow the steps below to set up the EOS project.

## Clone the Repository

```bash
git clone https://github.com/your-username/EOS.git
cd EOS
```

---

## Hardware Setup

Assemble the hardware components according to the wiring diagram.

Required hardware:

- ESP32-CAM (AI Thinker)
- OV2640 Camera
- Arduino Nano
- DFPlayer Mini
- Speaker
- FTDI Programmer
- Power Supply
- Jumper Wires

After wiring the system, upload the firmware to each microcontroller.

---

## Cloud Configuration

Configure the cloud services before running the system.

### Google Drive

Create a dedicated Google Drive folder for storing captured images.

---

### Google Sheets

Create a Google Sheet to automatically store image metadata.

Each captured image will generate a new record including:

- Capture Date
- Capture Time
- Upload Timestamp
- Google Drive Link
- Upload Status

---

### Google Apps Script

Deploy the Google Apps Script project and configure the following services:

- Google Drive API
- Google Sheets API

After deployment, copy the Web App URL into the dashboard configuration.

---

## Dashboard Configuration

Open the dashboard configuration file and update the required settings.

Example:

```javascript
const CONFIG = {
    ESP32_IP: "192.168.1.100",
    GOOGLE_SCRIPT_URL: "YOUR_GOOGLE_APPS_SCRIPT_URL",
    AUTO_UPLOAD: true
};
```

---

## ESP32 Configuration

Update your Wi-Fi credentials before uploading the firmware.

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

Compile and upload the project using the Arduino IDE.

---

# 🚀 Getting Started

Power on the hardware.

Wait until the ESP32-CAM connects to the Wi-Fi network.

Open the dashboard using your browser.

Enter the secure PIN.

Access the control panel.

Capture an image.

Verify that:

- The image appears in the dashboard.
- The image is uploaded to Google Drive.
- A new row is added to Google Sheets.

Your EOS system is now operational.

---

# 📋 Project Modules

The EOS project consists of several independent modules.

## Hardware Module

Responsible for:

- Image acquisition
- Audio output
- Embedded communication

---

## Dashboard Module

Responsible for:

- User authentication
- Camera control
- Device monitoring
- Image preview
- Cloud synchronization

---

## Cloud Module

Responsible for:

- Image storage
- Metadata logging
- Backup management

---

## AI Module

Responsible for:

- Image analysis
- Object detection
- Future intelligent features

---

# 📈 Future Improvements

EOS has been designed to support future expansion.

Planned improvements include:

- Real-time object detection
- Live video analysis
- OCR (Text Recognition)
- Face Recognition
- Currency Recognition
- Indoor Navigation
- Outdoor GPS Navigation
- Voice Commands
- Emergency Assistance
- Mobile Application
- Battery Monitoring
- Offline AI Processing
- Edge AI Optimization
- Custom AI Model Training
- Cloud Analytics Dashboard
- Multi-language Voice Feedback

---

# 🧪 Testing

Before deployment, verify the following:

- ESP32-CAM successfully connects to Wi-Fi.
- Camera captures images correctly.
- Dashboard authentication works.
- Flash LED responds correctly.
- Images upload to Google Drive.
- Google Sheets records every capture.
- Cloud synchronization completes successfully.

---

# 🤝 Contributors

This project was developed by the EOS Team.

<table>
<tr>
<td align="center">

### Ehab Amen

Project Lead

Embedded Systems

Backend Development

System Architecture

</td>

<td align="center">

### Mohamed Taha

Embedded Systems

Hardware Development

</td>

<td align="center">

### Mohamed Essameldin

Artificial Intelligence

Computer Vision

</td>

</tr>

<tr>

<td align="center">

### Aml Yaser

Dashboard Development

Cloud Integration

</td>

<td align="center">

### Norhan Mansour

Testing

Documentation

</td>

<td align="center">

### Yousef Mohamed

System Integration

Quality Assurance

</td>

</tr>
</table>

---

# 📄 License

This project is licensed under the MIT License.

See the LICENSE file for additional information.

---

# 🙏 Acknowledgments

The EOS team would like to express sincere appreciation to everyone who contributed to the development of this project.

Special thanks to:

- Faculty members
- Teaching assistants
- Open-source contributors
- Arduino Community
- ESP32 Community
- Google Cloud Platform
- Computer Vision Community

Their resources, documentation, and continuous support played a significant role in the successful development of this project.

---

# ⭐ Support the Project

If you find EOS useful, please consider giving this repository a ⭐ on GitHub.

Your support helps improve the project and encourages future development.

---

<div align="center">

## Thank You

**EOS**

### AI-Powered Smart Assistive Glasses for the Visually Impaired

Made with ❤️ by the EOS Team

</div>
