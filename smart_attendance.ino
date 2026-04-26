#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* pythonServer = "http://192.168.68.101:5000/process";

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#define PN532_IRQ (4)
#define PN532_RESET (15)
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

WebServer server(80);

String lastUser = "READY";
String lastStatus = "WAITING";
String systemID = "CPI-AUTO-01";
String attendanceLog = ""; 

struct Student {
  uint8_t uid[4];
  String name;
  String id;
};

Student students[5] = {
  {{0xD1, 0xC2, 0x35, 0x02}, "Nayeemur Rahman", "855764"},
  {{0x83, 0x3B, 0xD4, 0x12}, "Chandan Sarkar", "855773"},
  {{0x73, 0x9B, 0x8F, 0x11}, "Antu Ray", "855723"},
  {{0x13, 0x74, 0xAC, 0x12}, "Samira Akter", "855784"},
  {{0xD3, 0xC3, 0xDD, 0x2C}, "Jannat Islam", "2526010"}
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
  if(isResult) {
    u8g2.print("STATUS: ");
    u8g2.print(line2);
  } else {
    u8g2.print(line2);
  }
  u8g2.sendBuffer();
}

void handleRoot() {
  String s = "<!DOCTYPE html><html><head><meta charset='UTF-8'><script src='https://cdn.tailwindcss.com'></script>";
  s += "<style>body{background:#0d1117; color:#c9d1d9; font-family:sans-serif;} .glass{background:rgba(22,27,34,0.8); backdrop-filter:blur(10px); border:1px solid #30363d;}</style></head><body>";
  s += "<div class='container mx-auto p-4 md:p-10 max-w-6xl'>";
  s += "<div class='flex justify-between items-center mb-8 border-b border-gray-800 pb-5'>";
  s += "<div><h1 class='text-3xl font-black tracking-tighter text-white uppercase'>CPI Smart Attendance</h1>";
  s += "<p class='text-xs text-blue-400 font-mono'>AI-POWERED SECURITY PROTOCOL</p></div>";
  s += "<div class='text-right'><p class='text-xs text-gray-500'>DEVICE_ID</p><p class='font-mono text-white'>" + systemID + "</p></div></div>";
  s += "<div class='grid grid-cols-1 lg:grid-cols-3 gap-6'>";
  s += "<div class='lg:col-span-2 glass rounded-xl p-3 shadow-2xl'>";
  s += "<div class='flex justify-between items-center mb-2 px-2'><span class='text-[10px] text-red-500 animate-pulse'>● LIVE_FEED</span><span class='text-[10px] text-gray-500'>CAM_01_STREAM</span></div>";
  s += "<iframe src='http://192.168.68.107:81/stream' class='w-full h-[300px] md:h-[450px] rounded-lg bg-black'></iframe></div>";
  s += "<div class='space-y-6'>";
  s += "<div class='glass p-6 rounded-xl border-l-4 border-blue-500 shadow-lg'>";
  s += "<p class='text-gray-500 text-[10px] uppercase tracking-widest mb-1'>Last Scanned User</p><h2 class='text-xl font-bold text-white'>" + lastUser + "</h2></div>";
  String borderColor = (lastStatus == "Valid") ? "border-green-500" : "border-red-500";
  String textColor = (lastStatus == "Valid") ? "text-green-400" : "text-red-400";
  s += "<div class='glass p-6 rounded-xl border-l-4 " + borderColor + " shadow-lg'>";
  s += "<p class='text-gray-500 text-[10px] uppercase tracking-widest mb-1'>Dress Code Status</p><h2 class='text-xl font-bold " + textColor + "'>" + lastStatus + "</h2></div>";
  s += "<div class='glass p-4 rounded-xl h-[230px] overflow-hidden'>";
  s += "<p class='text-gray-500 text-[10px] uppercase mb-3 font-bold'>System Activity</p>";
  s += "<div class='text-[10px] font-mono space-y-1 text-gray-400'>";
  s += "<div>[INFO] System initialized...</div><div>[INFO] Wi-Fi stable...</div><div>[SCAN] Waiting for NFC card...</div>";
  s += "</div></div></div>";
  s += "<div class='lg:col-span-3 glass mt-6 rounded-xl p-6 shadow-xl'>";
  s += "<h3 class='text-white font-bold mb-4 flex items-center'><span class='mr-2 italic text-blue-500 font-black'>//</span> ATTENDANCE HISTORY LOGS</h3>";
  s += "<div class='overflow-x-auto'><table class='w-full text-left text-sm'>";
  s += "<thead class='text-gray-500 border-b border-gray-800'><tr><th class='pb-3 px-2'>STUDENT NAME</th><th class='pb-3 px-2 text-center'>DRESS STATUS</th><th class='pb-3 px-2 text-right'>TIMESTAMP</th></tr></thead>";
  s += "<tbody class='text-gray-300'>";
  s += attendanceLog; 
  s += "</tbody></table>";
  if(attendanceLog == "") s += "<p class='text-center text-gray-600 py-10 italic'>No records found. Waiting for first scan...</p>";
  s += "</div></div></div>";
  s += "<div class='mt-10 mb-5 text-center text-gray-600 text-[10px] tracking-widest uppercase'>© 2026 CPI Robotics Unit | All Systems Secured</div></div>";
  s += "<script>setInterval(()=>location.reload(), 3000);</script></body></html>";
  server.send(200, "text/html", s);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  u8g2.begin();
  drawUI("CPI SYSTEM", "INITIALIZING...", "CONNECTING WIFI");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  nfc.begin();
  nfc.SAMConfig();
  server.on("/", handleRoot);
  server.begin();
  drawUI("CPI ATTENDANCE", "SYSTEM READY", "WAITING FOR CARD");
}

void loop() {
  server.handleClient();
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
    for (int i = 0; i < 5; i++) {
      if (memcmp(uid, students[i].uid, 4) == 0) {
        lastUser = students[i].name;
        drawUI("PROCESSING AI", lastUser, "CHECKING DRESS...");
        HTTPClient http;
        http.begin(pythonServer);
        http.addHeader("Content-Type", "application/json");
        String json = "{\"name\":\"" + students[i].name + "\",\"id\":\"" + students[i].id + "\"}";
        int httpCode = http.POST(json);
        if(httpCode > 0) lastStatus = http.getString();
        else lastStatus = "SERVER ERR";
        http.end();
        String statusColor = (lastStatus == "Valid") ? "text-green-400" : "text-red-400";
        String newRow = "<tr class='border-b border-gray-800'><td class='py-3 px-2 font-bold text-white'>" + lastUser + "</td>";
        newRow += "<td class='py-3 px-2 text-center " + statusColor + "'>" + lastStatus + "</td>";
        newRow += "<td class='py-3 px-2 text-right text-gray-500 font-mono'>JUST NOW</td></tr>";
        attendanceLog = newRow + attendanceLog; 
        drawUI("SCAN COMPLETE", lastUser, lastStatus, true);
        delay(3000);
        drawUI("CPI ATTENDANCE", "SYSTEM READY", "WAITING FOR CARD");
        break;
      }
    }
  }
}
