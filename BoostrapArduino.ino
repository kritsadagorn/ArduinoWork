#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// กำหนด SSID และ Password ของ Wi-Fi
const char* ssid = "AQ";
const char* password = "12345678";

// ใช้ LED_BUILTIN แทนการกำหนดขา GPIO โดยตรง
const int ledPin = LED_BUILTIN;

// สร้างอินสแตนซ์ของเซิร์ฟเวอร์เว็บ
AsyncWebServer server(80);

// เก็บ HTML ไว้ในโปรแกรม memory (PROGMEM)
const char Main_page[] PROGMEM = R"====(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED Control</title>
  <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css">
  <style>
    body {
      background-color: lightblue; 
      display: flex;
      justify-content: center;
      align-items: center;
      height: 50vh;
      margin: 0;
    }
    .container {
      text-align: center;
    }
    h1 {
      font-family: 'Arial', sans-serif;
      margin-top: 0;
    }
    .mt-3, .mt-5, h3, form {
      margin-top: 1rem;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>LED Control</h1>
    <div class="mt-3">
      <a href="/H" class="btn btn-primary">Turn LED ON</a>
      <a href="/L" class="btn btn-danger">Turn LED OFF</a>
    </div>
    <div class="mt-3">
      <h3>Set Timer:</h3>
      <form action="/timerOn" method="get" class="form-inline justify-content-center">
        <div class="form-group mx-sm-3 mb-2">
          <label for="timeOn" class="sr-only">Set Time</label>
          <input type="number" class="form-control" id="timeOn" name="time" placeholder="Seconds">
        </div>
        <button type="submit" class="btn btn-success mb-2">Turn LED ON for set time</button>
      </form>
      <form action="/timerOff" method="get" class="form-inline justify-content-center">
        <div class="form-group mx-sm-3 mb-2">
          <label for="timeOff" class="sr-only">Set Time</label>
          <input type="number" class="form-control" id="timeOff" name="time" placeholder="Seconds">
        </div>
        <button type="submit" class="btn btn-warning mb-2">Turn LED OFF for set time</button>
      </form>
    </div>
  </div>
</body>
</html>
)====";


// ตัวแปรสำหรับจัดการเวลา
unsigned long timerStart = 0;
unsigned long timerDuration = 0;
bool timerActive = false;
bool timerOn = false;

void setup() {
  Serial.begin(115200); // เริ่มต้น Serial เพื่อใช้ในการ debug

  pinMode(ledPin, OUTPUT); // ตั้งค่า GPIO เป็น OUTPUT
  digitalWrite(ledPin, HIGH); // ตั้งค่าเริ่มต้นให้ LED ปิด (HIGH)

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // เริ่มต้นการเชื่อมต่อ Wi-Fi

  // รอจนกว่า Wi-Fi จะเชื่อมต่อสำเร็จ
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // แสดง IP address ของ ESP8266

  // กำหนดเส้นทาง URL สำหรับเปิดและปิด LED
  server.on("/H", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW); // เปิด LED (LOW)
    request->send_P(200, "text/html", Main_page); // ใช้ send_P เพื่อส่ง HTML จาก PROGMEM
    Serial.println("LED ON");
  });

  server.on("/L", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH); // ปิด LED (HIGH)
    request->send_P(200, "text/html", Main_page); // ใช้ send_P เพื่อส่ง HTML จาก PROGMEM
    Serial.println("LED OFF");
  });

  // ตั้งเวลาเปิด LED
  server.on("/timerOn", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("time")) {
      String timeParam = request->getParam("time")->value();
      timerDuration = timeParam.toInt() * 1000;
      timerStart = millis();
      timerActive = true;
      timerOn = true;
      digitalWrite(ledPin, LOW); // เปิด LED (LOW)
      Serial.print("LED ON for ");
      Serial.print(timerDuration / 1000);
      Serial.println(" seconds");
    }
    request->send_P(200, "text/html", Main_page); // ใช้ send_P เพื่อส่ง HTML จาก PROGMEM
  });

  // ตั้งเวลาปิด LED
  server.on("/timerOff", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("time")) {
      String timeParam = request->getParam("time")->value();
      timerDuration = timeParam.toInt() * 1000;
      timerStart = millis();
      timerActive = true;
      timerOn = false;
      digitalWrite(ledPin, HIGH); // ปิด LED (HIGH)
      Serial.print("LED OFF for ");
      Serial.print(timerDuration / 1000);
      Serial.println(" seconds");
    }
    request->send_P(200, "text/html", Main_page); // ใช้ send_P เพื่อส่ง HTML จาก PROGMEM
  });

  // กำหนดเส้นทาง URL หลัก
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", Main_page); // ใช้ send_P เพื่อส่ง HTML จาก PROGMEM
  });

  // เริ่มต้น Web Server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // ตรวจสอบเวลาเพื่อเปิดหรือปิด LED
  if (timerActive && (millis() - timerStart >= timerDuration)) {
    timerActive = false;
    if (timerOn) {
      digitalWrite(ledPin, HIGH); // ปิด LED หลังจากเวลาที่กำหนด
      Serial.println("LED OFF after timer");
    } else {
      digitalWrite(ledPin, LOW); // เปิด LED หลังจากเวลาที่กำหนด
      Serial.println("LED ON after timer");
    }
  }
}