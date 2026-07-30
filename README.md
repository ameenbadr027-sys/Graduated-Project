<div align="center">

<img src="https://img.shields.io/badge/Project-EOS-black?style=for-the-badge" alt="EOS"/>

# EOS

### AI-Powered Smart Assistive Glasses for the Visually Impaired

<p>
  <img src="https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=arduino&logoColor=white"/>
  <img src="https://img.shields.io/badge/ESP32--CAM-E7352C?style=flat-square&logo=espressif&logoColor=white"/>
  <img src="https://img.shields.io/badge/Artificial%20Intelligence-FF6F00?style=flat-square&logo=tensorflow&logoColor=white"/>
  <img src="https://img.shields.io/badge/Bluetooth%20Audio-0082FC?style=flat-square&logo=bluetooth&logoColor=white"/>
  <img src="https://img.shields.io/badge/Google%20Drive-4285F4?style=flat-square&logo=googledrive&logoColor=white"/>
  <img src="https://img.shields.io/badge/Google%20Sheets-34A853?style=flat-square&logo=googlesheets&logoColor=white"/>
</p>

> **Transforming the surrounding world into meaningful spoken awareness through Artificial Intelligence and Embedded Vision.**

*EOS is an intelligent wearable assistive platform that combines Embedded Systems, Computer Vision, Artificial Intelligence, IoT, and Cloud Computing to improve environmental awareness and independent mobility for visually impaired individuals.*

<br>

**⚡ Edge Computing • 🧠 AI Vision • 📡 IoT Connectivity • 🔊 Voice Assistance • ☁️ Cloud Integration**

<br>

**[▶ Obstacle Detection Demo](https://wokwi.com/projects/461059082332944385)** • **[▶ Camera Module Demo](https://wokwi.com/projects/450003284696400897)**

</div>

---

# 📖 Overview

EOS (Eye-Oriented Smart Assistant) is an AI-powered wearable platform designed to enhance environmental perception for visually impaired individuals.

The system combines embedded electronics, computer vision, cloud technologies, and intelligent voice interaction into a unified assistive solution capable of understanding the surrounding environment in real time.

A dedicated embedded subsystem continuously detects nearby obstacles with minimal latency, while an ESP32-CAM captures images that are securely transferred to an AI server for advanced visual analysis, including object detection, optical character recognition (OCR), and face recognition.

To improve safety and accessibility, a secure web dashboard enables an authorized companion to remotely monitor the camera, capture images, configure camera settings, and automatically synchronize every captured frame with Google Drive while recording its metadata inside Google Sheets.

EOS follows a modular architecture, allowing future AI models and hardware components to be integrated without redesigning the overall system.

---

# ✨ Key Features

- ⚡ **Real-Time Obstacle Detection** using an HC-SR04 ultrasonic sensor with immediate buzzer feedback.
- 📷 **Remote ESP32-CAM Management** through a secure cloud dashboard.
- 🔐 **PIN-Protected Dashboard Authentication** to prevent unauthorized access.
- 🎛 **Live Camera Configuration** including brightness, exposure, white balance, quality, and flash control.
- 📸 **Remote Image Capture** with instant preview.
- ☁️ **Automatic Google Drive Backup** for every captured image.
- 📊 **Automatic Google Sheets Logging** with timestamps and image metadata.
- 🧠 **AI Scene Understanding** through Object Detection, OCR, and Face Recognition.
- 🔊 **Bluetooth Voice Feedback** for recognized objects.
- 📡 **Cloud-Based Monitoring** for remote assistance.
- 🏗 **Modular Edge-to-Cloud Architecture** for future scalability.

---

# ⚙ System Operation

EOS operates through two complementary processing layers.

### Edge Layer

The wearable hardware continuously monitors the user's surroundings using the HC-SR04 ultrasonic sensor.

Whenever an obstacle enters the predefined safety distance, the Arduino immediately activates the buzzer, ensuring obstacle awareness without relying on internet connectivity or cloud services.

```cpp
distance = duration * 0.034 / 2;

if (distance > 0 && distance < 10) {
    digitalWrite(buzzerPin, HIGH);
}
```

---

### AI Layer

Whenever visual analysis is required, an authorized companion can remotely capture an image using the secure web dashboard.

The captured image is automatically:

1. Captured by the ESP32-CAM.
2. Uploaded to the AI server.
3. Analyzed using computer vision models.
4. Converted into natural language.
5. Delivered to the user through Bluetooth audio.
6. Archived in Google Drive.
7. Logged inside Google Sheets.

---

# 🤖 AI Processing Pipeline

```text
ESP32-CAM
      │
      ▼
Capture Image
      │
      ▼
Upload to AI Server
      │
      ▼
Image Processing
      │
      ▼
Object Detection
OCR
Face Recognition
      │
      ▼
Generate Natural Description
      │
      ▼
Bluetooth Voice Feedback
      │
      ▼
Google Drive Archive
      │
      ▼
Google Sheets Logging
```

---

# 🏗 System Architecture

EOS adopts a hybrid **Edge-to-Cloud Architecture**.

Critical operations requiring immediate response, such as obstacle detection, are executed locally on the wearable device, while computationally intensive vision tasks are performed on a dedicated AI server.

This design achieves both low-latency responsiveness and advanced visual intelligence while maintaining a lightweight embedded platform.

<p align="center">
<img width="638" height="695" alt="Screenshot 2026-07-30 030059" src="https://github.com/user-attachments/assets/4b4443dd-389e-44a3-8d6a-254c663fa5ad" />

</p>

---

# 🛠 Technology Stack

| Category | Technologies |
|------------|--------------------------------------------|
| Embedded Systems | Arduino Nano, ESP32-CAM |
| Camera | OV2640 |
| Sensors | HC-SR04 Ultrasonic Sensor |
| Artificial Intelligence | Object Detection, OCR, Face Recognition |
| Backend | PHP |
| Frontend | HTML, CSS, JavaScript |
| Cloud Services | Google Drive, Google Sheets, Google Apps Script |
| Communication | UART, Wi-Fi, Bluetooth |
| Development Tools | Arduino IDE, VS Code, Git, GitHub |

---

# 🌟 Why EOS?

EOS is more than a graduation project—it is an integrated assistive platform that demonstrates the convergence of Embedded Systems, Artificial Intelligence, Computer Vision, Cloud Computing, IoT, and Web Technologies.

Rather than simply detecting nearby obstacles, EOS aims to improve environmental understanding by converting visual information into meaningful spoken guidance, helping visually impaired users navigate daily life with greater confidence, safety, and independence.

---

# 🚀 Future Roadmap

- [ ] Edge AI inference directly on the ESP32.
- [ ] Real-time video object detection.
- [ ] Indoor navigation assistance.
- [ ] GPS outdoor navigation.
- [ ] Currency recognition.
- [ ] Smart OCR for documents.
- [ ] Emergency SOS module.
- [ ] Dedicated Android & iOS application.
- [ ] Multi-language voice interaction.
- [ ] Custom wearable PCB.
- [ ] Battery management system.
- [ ] Offline AI capabilities.

---

<div align="center">

## Engineering Technology That Restores Confidence Through Intelligence

Designed and developed as an intelligent assistive platform integrating **Embedded Systems**, **Artificial Intelligence**, **Computer Vision**, **Cloud Computing**, and **IoT** technologies.

### ⭐ If you found this project valuable, consider giving it a Star!

</div>
