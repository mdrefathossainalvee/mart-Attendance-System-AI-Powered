import cv2
import csv
import datetime
from flask import Flask, request
from ultralytics import YOLO

app = Flask(__name__)

# Load Model
try:
    model = YOLO('best.pt')
except Exception as e:
    print(f"Error: {e}")

CAM_URL = "http://192.168.68.107:81/stream"

def save_attendance(name, student_id, status):
    try:
        with open('attendance_log.csv', mode='a', newline='') as file:
            writer = csv.writer(file)
            time_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            writer.writerow([time_now, name, student_id, status])
    except:
        pass

@app.route('/process', methods=['POST'])
def process_ai():
    try:
        data = request.json
        name = data.get('name', 'Unknown')
        student_id = data.get('id', '0000')
        
        cap = cv2.VideoCapture(CAM_URL)
        ret, frame = cap.read()
        cap.release()

        if not ret or frame is None:
            return "Camera Error", 500

        results = model(frame, conf=0.25)
        
        is_dressed = False
        for r in results:
            for c in r.boxes.cls:
                class_name = model.names[int(c)].lower()
                if 'white shirt' in class_name:
                    is_dressed = True
                    break

        status = "Valid" if is_dressed else "Invalid"
        save_attendance(name, student_id, status)
        print(f"Detected {name}: {status}")
        return status
    except Exception as e:
        return str(e), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)
