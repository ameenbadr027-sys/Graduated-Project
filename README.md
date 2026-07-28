<div align="center">

<img src="https://img.shields.io/badge/Project-EOS-black?style=for-the-badge" alt="EOS"/>

<h1>EOS</h1>
<h3>Smart Assistive Glasses for the Visually Impaired</h3>

<p>
  <img src="https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=arduino&logoColor=white" />
  <img src="https://img.shields.io/badge/ESP32--CAM-E7352C?style=flat-square&logo=espressif&logoColor=white" />
  <img src="https://img.shields.io/badge/AI%20Powered-FF6F00?style=flat-square&logo=tensorflow&logoColor=white" />
  <img src="https://img.shields.io/badge/Bluetooth%20Audio-0082FC?style=flat-square&logo=bluetooth&logoColor=white" />
  <img src="https://img.shields.io/badge/Google%20Drive-4285F4?style=flat-square&logo=googledrive&logoColor=white" />
</p>

<p><i>The user feels their surroundings through sound and touch. Their companion sees it through AI.</i></p>

<br>

**[▶ Obstacle Detection Demo](https://wokwi.com/projects/461059082332944385)** &nbsp;·&nbsp; **[▶ Camera & AI Demo](https://wokwi.com/projects/450003284696400897)**

</div>

<br>

---

## The Idea

EOS gives visually impaired users two layers of awareness at once. Up close, an ultrasonic sensor and buzzer warn them instantly of anything in their path. Further out, a camera captures their surroundings, runs it through an AI model, and speaks the result straight back to them over Bluetooth — naming the object or person right in front of them.

A connected companion gets the same picture through a web dashboard, able to trigger captures remotely and see exactly what the AI detected. Two devices, one shared understanding of what's in front of the user.

<br>

## ✨ Features

- 🔊 **Instant obstacle alerts** — an ultrasonic sensor triggers a buzzer the moment something enters the 10cm danger zone
- 📷 **Remote camera control** — a companion can capture what the user is facing from a simple web dashboard
- 🧠 **AI scene analysis** — every captured image is processed for object detection, text (OCR), and face recognition
- 🗣️ **Spoken feedback for the user** — the AI's result is announced out loud over Bluetooth audio, naming the object or person detected
- ☁️ **Automatic cloud backup** — every capture is saved straight to Google Drive

<br>

## How It Works

<table>
<tr>
<td width="50%" valign="top">

### 🔊 Close Range — Arduino

An HC-SR04 sensor continuously scans the space ahead. The moment an obstacle enters the 10cm danger zone, the buzzer fires instantly.

```cpp
distance = duration * 0.034 / 2;

if (distance > 0 && distance < 10) {
  digitalWrite(buzzerPin, HIGH);
}
```

</td>
<td width="50%" valign="top">

### 📷 Wider Awareness — ESP32-CAM

A tap on the web dashboard triggers a photo capture. The AI identifies what's in frame, and the result is spoken aloud to the user over Bluetooth.

```
Capture → AI Analysis → Spoken to User → Saved to Drive
```

</td>
</tr>
</table>

<br>

## Architecture


<img width="1356" height="910" alt="image" src="https://github.com/user-attachments/assets/e2b6d2f1-7b35-405e-a0d1-c9a418f3891c" />


<br>

## 🎓 Why This Project

EOS was built as a graduation project — not just to prove technical skill across embedded systems, computer vision, AI, and web development, but to design something that solves a real problem for a real person, and gives them independence in the moment, not just data after the fact.

<br>

## 🚀 What's Next

- [ ] Merge both modules into a single wearable board
- [ ] Real-time video instead of single-shot capture
- [ ] Dedicated mobile app for the companion
- [ ] Multi-language spoken feedback

<br>

<div align="center">
<i>Built with care, for people who need it most.</i>
</div>
