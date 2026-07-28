<div align="center">

<img src="https://img.shields.io/badge/Project-EOS-black?style=for-the-badge" alt="EOS"/>

# EOS

### AI-Powered Smart Assistive Glasses for the Visually Impaired

<p>
  <img src="https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=arduino&logoColor=white"/>
  <img src="https://img.shields.io/badge/ESP32--CAM-E7352C?style=flat-square&logo=espressif&logoColor=white"/>
  <img src="https://img.shields.io/badge/AI%20Powered-FF6F00?style=flat-square&logo=tensorflow&logoColor=white"/>
  <img src="https://img.shields.io/badge/Bluetooth%20Audio-0082FC?style=flat-square&logo=bluetooth&logoColor=white"/>
  <img src="https://img.shields.io/badge/Google%20Drive-4285F4?style=flat-square&logo=googledrive&logoColor=white"/>
</p>

> **An intelligent wearable assistant that transforms the surrounding world into real-time spoken awareness.**

*Empowering independent navigation through embedded systems, computer vision, artificial intelligence, and natural voice feedback.*

<br>

**⚡ Real-Time Detection • 🧠 AI Vision • 📡 IoT Connectivity • 🔊 Voice Guidance • ☁️ Cloud Storage**

<br>

**[▶ Obstacle Detection Demo](https://wokwi.com/projects/461059082332944385)**  • 
**[▶ Camera & AI Demo](https://wokwi.com/projects/450003284696400897)**

</div>

---

# Overview

EOS is an intelligent wearable assistance platform designed to improve environmental awareness for visually impaired individuals.

The system combines embedded electronics, computer vision, artificial intelligence, wireless communication, and cloud technologies to transform visual information into meaningful spoken guidance.

Nearby obstacles are detected instantly through dedicated hardware with virtually zero latency, while distant scenes are analyzed using AI to recognize objects, identify people, and extract readable text.

At the same time, a companion can remotely monitor the user's surroundings through a lightweight web dashboard, providing an additional layer of safety whenever assistance is needed.

---

# ✨ Key Features

* ⚡ **Real-Time Obstacle Detection** using an ultrasonic sensor for immediate hazard awareness.
* 📷 **Remote Camera Control** through a web dashboard accessible by a companion.
* 🧠 **AI-Powered Scene Understanding** including Object Detection, OCR, and Face Recognition.
* 🔊 **Natural Voice Feedback** delivered directly to the user via Bluetooth audio.
* ☁️ **Automatic Cloud Backup** with every captured image securely stored on Google Drive.
* 📡 **Wireless Communication** between hardware modules, cloud services, and dashboard.
* 🏗️ **Modular Architecture** designed for future expansion and AI upgrades.

---

# How It Works

<table>
<tr>

<td width="50%" valign="top">

### ⚡ Edge Awareness

The Arduino continuously monitors the environment using an HC-SR04 ultrasonic sensor.

Whenever an obstacle enters the predefined safety zone (10 cm), the buzzer immediately alerts the user without requiring any network connection or AI processing.

```cpp
distance = duration * 0.034 / 2;

if (distance > 0 && distance < 10) {
    digitalWrite(buzzerPin, HIGH);
}
```

</td>

<td width="50%" valign="top">

### 🧠 AI Vision Pipeline

The companion can remotely trigger the ESP32-CAM to capture an image.

The captured frame is securely uploaded to the AI server where it undergoes:

* Object Detection
* OCR
* Face Recognition

The generated result is converted into speech, transmitted via Bluetooth, and finally archived to Google Drive.

</td>

</tr>
</table>

---

# AI Processing Flow

```text
Capture Image
      │
      ▼
Upload to AI Server
      │
      ▼
Object Detection
OCR
Face Recognition
      │
      ▼
Generate Speech
      │
      ▼
Bluetooth Audio Feedback
      │
      ▼
Cloud Storage + Web Dashboard
```

---

# System Architecture

EOS follows a distributed Edge-to-Cloud architecture.

Time-critical obstacle detection executes locally on the wearable hardware, while computationally intensive computer vision tasks are processed on an external AI server.

This hybrid architecture delivers both immediate responsiveness and advanced visual intelligence.

<img width="1356" height="910" alt="image" src="https://github.com/user-attachments/assets/acaf53f5-ca13-422e-bbd6-6e749b7fab57" />


---

# Tech Stack

| Category         | Technologies                            |
| ---------------- | --------------------------------------- |
| Embedded Systems | Arduino, ESP32-CAM                      |
| Sensors          | HC-SR04 Ultrasonic Sensor               |
| AI               | Object Detection, OCR, Face Recognition |
| Communication    | WiFi, Bluetooth                         |
| Backend          | PHP                                     |
| Frontend         | HTML, CSS, JavaScript                   |
| Cloud Storage    | Google Drive                            |

---

# Why EOS?

EOS represents more than a graduation project.

It demonstrates the integration of Embedded Systems, IoT, Computer Vision, Artificial Intelligence, Cloud Computing, and Web Development into a unified assistive platform capable of solving a real-world accessibility challenge.

Rather than simply identifying objects, EOS was designed to enhance environmental awareness, improve independent mobility, and provide visually impaired users with greater confidence in navigating everyday life.

---

# 🚀 Roadmap

* [ ] Custom wearable PCB
* [ ] Edge AI inference directly on the device
* [ ] Real-time video understanding
* [ ] Dedicated mobile application
* [ ] Indoor navigation assistance
* [ ] Emergency SOS system
* [ ] Multi-language voice feedback

---

<div align="center">

## Engineering Technology That Restores Confidence Through Intelligence

Designed and developed as an assistive AI platform combining **Embedded Systems**, **IoT**, **Computer Vision**, **Artificial Intelligence**, and **Cloud Technologies**.

**⭐ If you found this project interesting, consider giving it a Star!**

</div>
