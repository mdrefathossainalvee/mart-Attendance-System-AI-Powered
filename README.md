# Smart Attendance System (AI-Powered)

An advanced, end-to-end IoT solution that integrates **NFC (Near Field Communication)** for identity authentication and **YOLOv8 AI Vision** for real-time dress code verification. This system is designed for professional institutions to automate attendance while ensuring compliance with dress code policies.

---

## Features
* **Dual-Factor Authentication:** NFC Card Identification + AI-based Dress Code Verification.
* **AI Vision Integration:** Custom-trained **YOLOv8** model to detect "white shirts" in real-time.
* **Dynamic Dashboard:** Modern, dark-themed web interface built with **Tailwind CSS** featuring a live camera stream and real-time logs.
* **OLED System Feedback:** 1.3" SH1106 OLED display for granular status updates (Scanning -> Processing -> Result).
* **Automated Data Logging:** Attendance records with timestamps and status are archived into a local CSV database.

---

## Hardware Wiring & Pin Mapping

The system utilizes a shared **I2C Bus** for both the OLED display and the NFC reader to maximize pin efficiency on the ESP32.

### 1. ESP32 to PN532 (NFC)
| PN532 Pin | ESP32 Pin | Type | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 5V | Power | External or 5V Pin |
| **GND** | GND | Ground | Common Ground |
| **SDA** | GPIO 21 | I2C | Data Line |
| **SCL** | GPIO 22 | I2C | Clock Line |
| **IRQ** | GPIO 4 | Input | Interrupt Request |
| **RST** | GPIO 15 | Output | Reset Pin |

### 2. ESP32 to 1.3" OLED (SH1106)
| OLED Pin | ESP32 Pin | Type | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 3.3V | Power | Logic Power |
| **GND** | GND | Ground | Common Ground |
| **SDA** | GPIO 21 | I2C | Shared I2C Data |
| **SCL** | GPIO 22 | I2C | Shared I2C Clock |

---

## Tech Stack
* **Microcontroller:** ESP32 (Main Controller) & ESP32-CAM (Vision)
* **AI Engine:** YOLOv8 (Ultralytics)
* **Backend Server:** Python Flask
* **Frontend UI:** HTML5, Tailwind CSS
* **Libraries:** `ultralytics`, `opencv-python`, `U8g2`, `Adafruit_PN532`

---

## Code Explanation

### 1. ESP32 Firmware (Edge Controller)
The ESP32 firmware manages the hardware layer. It constantly polls the PN532 for NFC tags. Upon detection, it updates the OLED display via the **U8g2 library** and sends a JSON POST request containing student details to the Python backend. It also hosts a web server to serve the real-time **Tailwind CSS dashboard**.

### 2. AI Backend Server (Decision Engine)
The Python server, built with **Flask**, acts as the processing hub. When it receives a request, it accesses the MJPEG stream from the **ESP32-CAM** using OpenCV. The frame is then passed to the **YOLOv8** model. If the model detects a `white shirt` with confidence > 0.25, it returns a "Valid" status. Finally, it logs the event into a CSV file.

---

## Installation & Setup

1.  **Clone the Repo:**
    ```bash
    git clone [https://github.com/yourusername/cpi-smart-attendance.git](https://github.com/yourusername/cpi-smart-attendance.git)
    cd cpi-smart-attendance
    ```
2.  **Setup Python Environment:**
    ```bash
    python3 -m venv venv
    source venv/bin/activate
    pip install flask ultralytics opencv-python
    python main.py
    ```
3.  **Flash ESP32:**
    * Open `.ino` file in Arduino IDE.
    * Update WiFi credentials and `pythonServer` IP.
    * Upload to ESP32 DevKit V1.

---

## Contributors
* **Md. Refat Hossain Alvee** - Team Lead & Software Architect (OneLastBit)
* **Institutional Context:** CPI Smart Attendance System Project
