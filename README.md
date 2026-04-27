# CPI Smart Attendance System (AI-Powered)

An advanced IoT and Artificial Intelligence integration designed to automate student attendance while enforcing institutional dress codes. The system uses **NFC** for identity verification and a custom-trained **YOLOv8 AI model** to detect compliance (White Shirt) in real-time.

---

## Why This Project?
Traditional attendance systems only verify "who" is entering, not "how" they are entering. This project adds a layer of institutional discipline by:
1.  **Ensuring Identity:** Using encrypted NFC tags.
2.  **Verifying Compliance:** Using Computer Vision to detect the mandatory white shirt.
3.  **Real-time Monitoring:** Updating a live web dashboard and local database simultaneously.

---

## Features
* **Dual-Factor Authentication:** NFC-based ID + AI-based Vision check.
* **Edge Computing & Cloud Hybrid:** ESP32 handles hardware while a Python server handles heavy AI inference.
* **Visual & Audio Feedback:** OLED status display, RGB LED indicators, and Buzzer alerts.
* **Cross-Platform Dashboard:** A responsive UI built with Tailwind CSS.
* **Automated Logging:** Saves data to `attendance_log.csv` with precise timestamps.

---

## System Architecture & Wiring

### **Hardware Components**
* **Main Controller:** ESP32 DevKit V1
* **Vision Module:** ESP32-CAM
* **NFC Reader:** PN532 Module
* **Display:** 1.3" SH1106 OLED (I2C)
* **Indicators:** Green LED (Success), Red LED (Failure), Active Buzzer

### **Pin Mapping**
| Component | ESP32 Pin | Mode | Description |
| :--- | :--- | :--- | :--- |
| **NFC SDA** | GPIO 21 | I2C | Shared Data Line |
| **NFC SCL** | GPIO 22 | I2C | Shared Clock Line |
| **OLED SDA** | GPIO 21 | I2C | Shared Data Line |
| **OLED SCL** | GPIO 22 | I2C | Shared Clock Line |
| **Green LED** | GPIO 18 | Output | Valid Entry |
| **Red LED** | GPIO 19 | Output | Invalid Entry |
| **Buzzer** | GPIO 5 | Output | Audio Alert |



---

## How It Works (The Logic)

1.  **Scanning:** The student taps their NFC card. The ESP32 extracts the Unique ID (UID).
2.  **Request:** ESP32 identifies the student locally and sends a JSON request to the Python Server.
3.  **Vision:** The Python Server connects to the ESP32-CAM's MJPEG stream, captures a frame, and runs it through the **YOLOv8** model.
4.  **Inference:** If the model detects a `white shirt` with > 25% confidence, it marks the entry as **Valid**.
5.  **Response:** The server returns the Status, Date, and Time to the ESP32.
6.  **Feedback:** * **Valid:** Green LED ON + Short Beep + OLED "Valid".
    * **Invalid:** Red LED ON + Long Beep + OLED "Invalid".



---

## Installation & Setup

### **1. Server Setup (Python)**

Follow these steps based on your Operating System:

#### **Windows**
1.  Install Python 3.10+ from [python.org](https://www.python.org/).
2.  Open Terminal/PowerShell:
    ```bash
    python -m venv venv
    .\venv\Scripts\activate
    pip install flask ultralytics opencv-python requests
    python main.py
    ```

#### **macOS**
1.  Open Terminal:
    ```bash
    python3 -m venv venv
    source venv/bin/activate
    pip install flask ultralytics opencv-python requests
    python3 main.py
    ```

#### **Ubuntu / Linux**
1.  Open Terminal:
    ```bash
    sudo apt update && sudo apt install python3-venv python3-pip
    python3 -m venv venv
    source venv/bin/activate
    pip install flask ultralytics opencv-python requests
    python3 main.py
    ```

### **2. Firmware Setup (Arduino)**
1.  Install **Arduino IDE**.
2.  Add ESP32 Board URL in Preferences.
3.  Install libraries: `U8g2`, `Adafruit_PN532`.
4.  Update the following variables in `cpi_smart_attendance.ino`:
    * `ssid`: Your WiFi Name.
    * `password`: Your WiFi Password.
    * `pythonServer`: Your Computer's Local IP (e.g., `http://192.168.0.105:5000/process`).
5.  Select **DOIT ESP32 DEVKIT V1** and Upload.

---

## Web Dashboard
The system hosts a local web server at the ESP32's IP address. It features:
* **Live Feed:** Real-time monitoring of the entrance.
* **Attendance Table:** Dynamic list showing Name, ID, Dept, Date, Status, and Time.
* **Auto-Refresh:** Updates every 3 seconds using JavaScript.



---

## Contributors
* **Md. Refat Hossain Alvee** - Team Lead & Systems Architect
* **Organization:** OneLastBit
* **Project Context:** CPI Smart Attendance Security Protocol

---
© 2026 NEXTRO Robotics. All Rights Reserved.
