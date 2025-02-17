#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// ตั้งค่าการเชื่อมต่อ WiFi
const char* ssid = "Sissasitorn_2.4G";
const char* password = "25124446";

// ตั้งค่า Thingspeak
const String thingSpeakURL = "http://api.thingspeak.com/update?api_key=";
String apiKey = "7F46IXV17BCTAW0L"; // เปลี่ยน API KEY

Adafruit_BMP280 bmp;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(1000); // Delay for serial monitor to initialize
  Serial.println("Starting up...");

  Wire.begin();

  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  connectWiFi();

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  float altitude = bmp.readAltitude(1013.25); // ค่าความดันมาตรฐานระดับน้ำทะเล 1013.25 hPa

  // Send data to ThingSpeak
  HTTPClient httpThingSpeak;
  String thingSpeakURLWithParams = thingSpeakURL + apiKey 
      + "&field1=" + String(temperature) 
      + "&field2=" + String(pressure)
      + "&field3=" + String(altitude);

  httpThingSpeak.begin(client, thingSpeakURLWithParams);
  int httpResponseCodeThingSpeak = httpThingSpeak.GET();

  if (httpResponseCodeThingSpeak > 0) {
    String responseThingSpeak = httpThingSpeak.getString();
    Serial.println("HTTP Response Code: " + String(httpResponseCodeThingSpeak));
    Serial.println("Response: " + responseThingSpeak);
  } else {
    Serial.print("Error on sending GET: ");
    Serial.println(httpResponseCodeThingSpeak);
  }
  httpThingSpeak.end();

  Serial.println("Going to sleep for 10 minutes...");
  ESP.deepSleep(2 * 60 * 1000000); // Sleep for 10 minutes
}

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // เพิ่มความอดทนให้กับการเชื่อมต่อ WiFi
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi");
  }
}

void loop() {
  // Loop is not used when using deep sleep
}
