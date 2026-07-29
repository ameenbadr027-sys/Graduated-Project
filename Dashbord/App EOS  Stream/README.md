# 📸 EOS Camera Dashboard

The EOS Camera Dashboard is the central control interface for the ESP32-CAM vision module used in the EOS Smart Assistive Glasses project. It provides a complete environment for configuring the OV2640 camera, monitoring live video, capturing images, managing device operations, and preparing frames for AI-based object detection.

<p align="center">
  <img width="789" height="875" alt="Screenshot 2026-07-29 142420" src="https://github.com/user-attachments/assets/9c9463d4-96e2-4e9d-b898-2ea83998afd1" />

</p>

## Dashboard Overview

The dashboard is divided into several functional sections, each responsible for a specific aspect of camera control and system monitoring.

### 🎥 Live Video Stream

The main panel displays a real-time MJPEG stream directly from the ESP32-CAM. This live preview allows developers to verify camera positioning, monitor image quality, and observe the environment before running object detection or capturing images.

---

### 🎛 Camera Configuration

The left control panel exposes the complete OV2640 camera configuration, allowing parameters to be changed instantly without reflashing the firmware.

Available settings include:

- Frame Size (Resolution)
- JPEG Compression Quality
- Brightness
- Contrast
- Saturation
- Sharpness
- White Balance
- Auto White Balance (AWB)
- Auto Exposure Control (AEC)
- Auto Gain Control (AGC)
- Gain Ceiling
- Lens Correction
- Horizontal Mirror
- Vertical Flip
- Special Image Effects
- Color Bar Test
- Face Detection
- Face Recognition

These controls help optimize image quality for different lighting conditions and improve the accuracy of the downstream AI vision pipeline.

---

### 📸 Image Acquisition

The dashboard allows users to capture high-resolution still images directly from the OV2640 camera. Captured frames can be used for:

- Object detection testing
- AI dataset collection
- Performance evaluation
- Debugging camera settings
- Image quality comparison

---

### 💡 Flash LED Control

The integrated ESP32-CAM flash LED can be switched on or off directly from the dashboard. This improves image visibility in low-light environments and ensures more consistent object detection performance.

---

### ☁ Cloud Backup

Captured images can be uploaded automatically to Google Drive through an integrated cloud service. This feature enables secure storage, remote access, and dataset management without requiring physical access to the device.

---

### 📊 System Status

The dashboard continuously monitors the ESP32-CAM and displays important runtime information, including:

- Device connectivity
- Network status
- Camera availability
- Stream state
- System uptime
- Active requests

This information helps verify that the hardware is operating correctly during deployment.

---

### 📝 System Console

A real-time console records all communication between the dashboard and the ESP32-CAM.

Typical log entries include:

- Stream initialization
- Camera configuration updates
- Image capture requests
- Flash operations
- Cloud upload status
- Device restart events
- API responses
- Error messages

The console greatly simplifies debugging and system monitoring during development.

---

### 🌐 Network Communication

The dashboard communicates with the ESP32-CAM using lightweight HTTP endpoints over the local Wi-Fi network. Every user action—such as changing camera settings, capturing images, or controlling the flash—is translated into HTTP requests handled by the ESP32 web server in real time.

---

### 🤖 Integration with AI Object Detection

The Camera Dashboard represents the first stage of the EOS computer vision pipeline.

Its primary responsibility is to acquire high-quality images from the OV2640 camera, optimize sensor parameters, and deliver reliable image frames to the AI object detection module. The processed detection results are then forwarded to the remaining EOS subsystems, where recognized objects can be converted into meaningful feedback for visually impaired users.
