import cv2, csv, datetime
from flask import Flask, request
from ultralytics import YOLO

app = Flask(__name__)
model = YOLO('best.pt')
CAM_URL = "http://192.168.68.107:81/stream"

def save_attendance(name, student_id, dept, status):
    with open('attendance_log.csv', mode='a', newline='') as file:
        writer = csv.writer(file)
        time_now = datetime.datetime.now().strftime("%I:%M:%S %p") # 12-hour format with AM/PM
        writer.writerow([time_now, name, student_id, dept, status])

@app.route('/process', methods=['POST'])
def process_ai():
    data = request.json
    name = data.get('name', 'Unknown')
    student_id = data.get('id', '0000')
    dept = data.get('dept', 'N/A')
    
    cap = cv2.VideoCapture(CAM_URL)
    ret, frame = cap.read()
    cap.release()

    if not ret or frame is None: return "Camera Error", 500

    results = model(frame, conf=0.25)
    is_dressed = any('white shirt' in model.names[int(c)].lower() for r in results for c in r.boxes.cls)

    status = "Valid" if is_dressed else "Invalid"
    save_attendance(name, student_id, dept, status)
    return status

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
