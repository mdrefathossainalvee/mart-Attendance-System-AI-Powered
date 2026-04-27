#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* pythonServer = "http://YOUR_MAC_IP:5000/process";

#define GREEN_LED 18
#define RED_LED 19
#define BUZZER 5

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#define PN532_IRQ (4)
#define PN532_RESET (15)
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

WebServer server(80);

String lastUser = "READY";
String lastStatus = "WAITING";
String attendanceLog = ""; 

struct Student {
  uint8_t uid[4];
  String name;
  String id;
  String dept;
};

Student students[5] = {
  {{0xD1, 0xC2, 0x35, 0x02}, "Nayeemur Rahman", "855764", "EEE"},
  {{0x83, 0x3B, 0xD4, 0x12}, "Chandan Sarkar", "855773", "CSE"},
  {{0x73, 0x9B, 0x8F, 0x11}, "Antu Ray", "855723", "Civil"},
  {{0x13, 0x74, 0xAC, 0x12}, "Samira Akter", "855784", "Textile"},
  {{0xD3, 0xC3, 0xDD, 0x2C}, "Jannat Islam", "2526010", "Computer"}
};

void drawUI(String header, String line1, String line2, bool isResult = false) {
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(5, 12, header.c_str());
  u8g2.drawHLine(0, 15, 128);
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(10, 35);
  u8g2.print(line1);
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.setCursor(10, 55);
  if(isResult) { u8g2.print("STATUS: "); u8g2.print(line2); }
  else { u8g2.print(line2); }
  u8g2.sendBuffer();
}

void triggerFeedback(bool isValid) {
  if (isValid) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(200); digitalWrite(BUZZER, LOW);
    delay(1000); digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(1000); digitalWrite(BUZZER, LOW);
    digitalWrite(RED_LED, LOW);
  }
}

void handleRoot() {
  String s = "<!DOCTYPE html><html><head><meta charset='UTF-8'><script src='https://cdn.tailwindcss.com'></script>";
  s += "<style>body{background:#0d1117; color:#c9d1d9; font-family:sans-serif;} .glass{background:rgba(22,27,34,0.8); backdrop-filter:blur(10px); border:1px solid #30363d;}</style></head><body>";
  s += "<div class='container mx-auto p-4 md:p-10 max-w-6xl'>";
  s += "<h1 class='text-3xl font-black text-white uppercase mb-8 border-b border-gray-800 pb-4'>CPI Smart Attendance</h1>";
  s += "<div class='grid grid-cols-1 lg:grid-cols-3 gap-6 mb-8'>";
  s += "<div class='lg:col-span-2 glass rounded-xl p-3'><iframe src='http://172.20.10.4:81/stream' class='w-full h-[350px] rounded-lg bg-black'></iframe></div>";
  s += "<div class='space-y-4'>";
  s += "<div class='glass p-6 rounded-xl border-l-4 border-blue-500'><p class='text-gray-500 text-xs uppercase'>User</p><h2 class='text-xl font-bold'>" + lastUser + "</h2></div>";
  String statusCol = (lastStatus == "Valid") ? "text-green-400" : "text-red-400";
  s += "<div class='glass p-6 rounded-xl border-l-4 border-gray-700'><p class='text-gray-500 text-xs uppercase'>Status</p><h2 class='text-xl font-bold " + statusCol + "'>" + lastStatus + "</h2></div>";
  s += "</div></div>";
  s += "<div class='glass rounded-xl p-6 shadow-xl'>";
  s += "<div class='overflow-x-auto'><table class='w-full text-left text-sm'>";
  s += "<thead class='text-gray-500 border-b border-gray-800'><tr>";
  s += "<th class='pb-3 px-2'>NAME</th><th class='pb-3'>ID</th><th class='pb-3'>DEPT</th><th class='pb-3'>DATE</th><th class='pb-3 text-center'>DRESS</th><th class='pb-3 text-right'>TIME</th>";
  s += "</tr></thead><tbody class='text-gray-300'>" + attendanceLog + "</tbody></table></div></div></div>";
  s += "<script>setInterval(()=>location.reload(), 3000);</script></body></html>";
  server.send(200, "text/html", s);
}

void setup() {
  pinMode(GREEN_LED, OUTPUT); pinMode(RED_LED, OUTPUT); pinMode(BUZZER, OUTPUT);
  Serial.begin(115200); Wire.begin(21, 22); u8g2.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  nfc.begin(); nfc.SAMConfig();
  server.on("/", handleRoot); server.begin();
  drawUI("SYSTEM", "READY", "WAITING...");
}

void loop() {
  server.handleClient();
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
    for (int i = 0; i < 5; i++) {
      if (memcmp(uid, students[i].uid, 4) == 0) {
        lastUser = students[i].name;
        drawUI("AI CHECK", lastUser, "PROCESSING...");
        
        HTTPClient http;
        http.begin(pythonServer);
        http.addHeader("Content-Type", "application/json");
        String json = "{\"name\":\"" + students[i].name + "\",\"id\":\"" + students[i].id + "\",\"dept\":\"" + students[i].dept + "\"}";
        
        int httpCode = http.POST(json);
        if(httpCode > 0) {
           String response = http.getString(); // Format: Status|Date|Time
           int firstPipe = response.indexOf('|');
           int lastPipe = response.lastIndexOf('|');
           
           lastStatus = response.substring(0, firstPipe);
           String logDate = response.substring(firstPipe + 1, lastPipe);
           String logTime = response.substring(lastPipe + 1);

           bool isValid = (lastStatus == "Valid");
           String statusColor = isValid ? "text-green-400" : "text-red-400";
           
           String row = "<tr class='border-b border-gray-800'><td class='py-3 px-2 font-bold text-white'>" + students[i].name + "</td>";
           row += "<td class='py-3'>" + students[i].id + "</td>";
           row += "<td class='py-3'>" + students[i].dept + "</td>";
           row += "<td class='py-3'>" + logDate + "</td>";
           row += "<td class='py-3 text-center " + statusColor + "'>" + lastStatus + "</td>";
           row += "<td class='py-3 text-right text-gray-500 font-mono'>" + logTime + "</td></tr>";
           attendanceLog = row + attendanceLog; 

           drawUI("RESULT", lastUser, lastStatus, true);
           triggerFeedback(isValid);
        } else {
           lastStatus = "SERVER ERR";
           drawUI("ERROR", "SERVER", "NOT RESPONDING");
        }
        http.end();
        delay(2000);
        drawUI("SYSTEM", "READY", "WAITING...");
        break;
      }
    }
  }
}
