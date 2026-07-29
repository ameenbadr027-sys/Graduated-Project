# AI Model — EOS AI Object Detection

**Custom YOLO-26m based object detection model for real-time scene understanding**

Part of the EOS Smart Assistive Glasses graduation project  
Faculty of Science, Zagazig University · 2025–2026

---

## 1. Model Overview

| Property | Details |
|----------|---------|
| **Name** | YOLO26m |
| **Type** | Single-Stage Object Detector |
| **Framework** | PyTorch (Ultralytics YOLO) |
| **Format** | `.pt` checkpoint |
| **Size** | ~43 MB |
| **Task** | Bounding box detection + class classification |

### What is YOLO26m?
YOLO26m is a **custom fine-tuned variant** of the YOLO (You Only Look Once) family. It follows the single-shot detection paradigm: the model processes the entire image in **one forward pass** to simultaneously predict:
- Object bounding boxes (coordinates)
- Objectness scores (confidence that an object exists)
- Class probabilities (what the object is)

This makes it extremely fast compared to two-stage detectors (like R-CNN), which is critical for real-time assistive applications.

---

## 2. Algorithm & Architecture

### 2.1 Core Algorithm: YOLO (You Only Look Once)

```
Input Image (640×640)
    │
    ▼
┌─────────────────┐
│  Backbone       │  ← Feature extraction (CSPDarknet-style)
│  (CNN layers)   │
└─────────────────┘
    │
    ▼
┌─────────────────┐
│  Neck (FPN/PAN) │  ← Multi-scale feature fusion
└─────────────────┘
    │
    ▼
┌─────────────────┐
│  Head           │  ← Detection: boxes + classes + confidence
└─────────────────┘
    │
    ▼
Output: [x, y, w, h, confidence, class_probs]
```

**Why single-stage?**
- **Speed:** One forward pass = one prediction
- **End-to-end:** No separate region proposal step
- **Real-time ready:** Optimized for near-real-time inference on CPU/GPU

### 2.2 Architecture Details

| Component | Function |
|-----------|----------|
| **Backbone** | Extracts hierarchical features from input image (edges → textures → objects) |
| **Neck (FPN/PAN)** | Fuses features from different scales so small and large objects are both detected well |
| **Detection Head** | Outputs final predictions: bounding boxes, objectness, and class labels |
| **Anchor-free design** | Predicts box centers directly (common in modern YOLO versions like YOLOv8) |

### 2.3 Post-Processing
- **NMS (Non-Maximum Suppression):** Removes duplicate detections of the same object
- **Confidence Thresholding:** Filters out low-confidence predictions

---

## 3. Training Data & Classes

### 3.1 Dataset
The model was trained on a curated dataset targeting **everyday assistive scenarios**:

| Category | Examples |
|----------|----------|
| **People** | person, pedestrian |
| **Furniture** | chair, table, bed, sofa, desk |
| **Vehicles** | car, bus, bicycle, motorcycle |
| **Street Objects** | traffic light, stop sign, fire hydrant |
| **Animals** | dog, cat, bird |
| **Everyday Items** | bottle, cup, book, phone, laptop, backpack |

*(Note: Exact class list depends on your training configuration. Verify with your training logs.)*

### 3.2 Data Format
- **Images:** JPG/PNG (typically 640×640 for training)
- **Labels:** YOLO format `.txt` files:
  ```
  <class_id> <x_center> <y_center> <width> <height>
  ```
  (All values normalized to [0, 1])

### 3.3 Data Augmentation (Typical)
- Random horizontal flip
- Mosaic augmentation
- MixUp
- HSV color jittering
- Random scaling and translation

---

## 4. Model Files Structure

```
ModelAI/
├── .data/              # Metadata and serialization info
├── data/               # Chunked tensor data (model parameters)
├── data.pkl            # Pickled Python objects (model architecture + state_dict)
├── .format_version     # PyTorch serialization format version
├── .storage_alignment  # Memory alignment info
├── byteorder           # Endianness (little-endian)
└── version             # PyTorch version used to save the model
```

> **Note:** These files together form the `.pt` checkpoint. Do not separate them — they must stay together for the model to load correctly.

---

## 5. Performance Metrics

| Metric | Value |
|--------|-------|
| **Precision** | ~0.864 |
| **Recall** | ~0.850 |
| **mAP@0.5** | ~92.8% |
| **mAP@0.5:0.95** | ~78.7% |
| **Inference Time** | ~190–280 ms/frame (CPU) |
| **Effective FPS** | ~3–6 FPS (including HTTP fetch + inference) |

*Metrics measured on project validation set. Re-measure on your own data after deployment.*

---

## 6. How to Use

### 6.1 Requirements

```bash
pip install ultralytics opencv-python numpy
```

### 6.2 Load the Model

```python
from ultralytics import YOLO

# Load the custom model
model = YOLO("weights/yolo26m.pt")

# Run inference on an image
results = model("path/to/image.jpg")

# Display results
results[0].show()
```

### 6.3 Run on Video / Live Stream

```python
import cv2
from ultralytics import YOLO

model = YOLO("weights/yolo26m.pt")
cap = cv2.VideoCapture(0)  # or video file path

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Run detection
    results = model(frame, conf=0.5)

    # Draw results
    annotated = results[0].plot()
    cv2.imshow("Detection", annotated)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 6.4 Export to Other Formats (Optional)

```python
# Export to ONNX for faster inference
model.export(format="onnx")

# Export to TensorRT (requires GPU)
model.export(format="engine")
```

---

## 7. Integration with EOS System

```text
[Hardware Layer]
  ESP32-CAM ──► Captures frames ──► HTTP JPEG stream

[AI Layer — This Model]
  PC/Laptop ──► Fetches frames ──► YOLO26m inference
            ──► Bounding boxes + labels
            ──► Text-to-Speech feedback
            ──► Visual overlay

[User Layer]
  User ──► Hears detected objects via pyttsx3
       ──► Sees annotated preview (optional)
```

---

## 8. Important Notes

| ⚠️ Warning | Details |
|------------|---------|
| **Large file** | This model is ~43 MB. Use Git LFS if storing in a repository. |
| **PyTorch version** | Load with the same or compatible PyTorch version used during training. |
| **Input size** | Default training resolution is typically 640×640. Resize inputs accordingly. |
| **Warm-up** | First inference may be slower due to model initialization. |

---

## 9. Citation & Attribution

This model is part of the **EOS — Smart Assistive Glasses** graduation project.

**Team:**
- Mohamed Ahmed Ali
- Youssef Mohamed Ali Ahmed
- Mohamed Essam Hessen
- Ehab Amen Mohamed
- Nourhan Mohamed Mansour
- Aml Yasser Othman Ibrahim

**Supervisor:** Dr. Zahraa Youssef  
**Institution:** Faculty of Science, Zagazig University · 2025–2026

Built with [Ultralytics YOLO](https://github.com/ultralytics/ultralytics).

---

## 10. License

This model is provided for academic and research purposes as part of the EOS graduation project.
