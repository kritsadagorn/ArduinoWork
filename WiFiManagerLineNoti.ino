#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

#define LINE_NOTIFY_TOKEN "eS0e4lHZ4YWMrhRh3QyRiSDyjSmzoOS0y3KkhCLim6b" // Replace with your LINE Notify token
const int GPIOPIN = 2; // LED Pins
String etatGpio = "OFF";
ESP8266WebServer server(80);

String getPage() {
  String page = "<html charset=UTF-8><head><meta name='viewport' content='width=device-width, initial-scale=1'/>";
  page += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>";
  page += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>";
  page += "<title>LED Control</title>";
  page += "<style>";
  page += ".container { background-color: rgba(0, 151, 215, 0.8); padding: 20px; border-radius: 15px; box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.1); max-width: 400px; width: 100%; margin-top: 20vh; }";
  page += ".text-center { color: white; }";
  page += ".btn { font-size: 1.2em; margin: 5px 0; transition: transform 0.2s, background-color 0.2s; }";
  page += ".btn:hover { transform: scale(1.1); }";
  page += ".btn:active { background-color: darkred; color: white; }";
  page += ".badge { font-size: 1em; }";
  page += ".badge-success { background-color: #d9534f; }"; // Changed to a shade of red
  page += ".badge-danger { background-color: #a94442; }"; // Changed to a darker shade of red
  page += "</style></head><body>";
  page += "<div class='container'>";
  page += "<div class='row'><div class='col-md-12'><h1 class='text-center'>LED Controller</h1><hr>";

  page += "<div class='row text-center'>";
  page += "<h4>D" + String(GPIOPIN) + " ";
  page += "<span class='badge " + String(etatGpio == "ON" ? "badge-success" : "badge-danger") + "'>" + etatGpio + "</span></h4></div>";
  page += "<div class='row'>";
  page += "<div class='col-xs-6 text-center'><form action='/' method='POST'><button type='submit' name='D" + String(GPIOPIN) + "' value='1' class='btn btn-success btn-block'>ON</button></form></div>";
  page += "<div class='col-xs-6 text-center'><form action='/' method='POST'><button type='submit' name='D" + String(GPIOPIN) + "' value='0' class='btn btn-danger btn-block'>OFF</button></form></div>";
  page += "</div><br>";

  page += "</div></div></body></html>";
  return page;
}


void handleRoot() {
  if (server.hasArg("D2")) {
    handleGPIO(server.arg("D2"));
  } else {
    server.send(200, "text/html", getPage());
  }
}

void handleGPIO(String value) {
  if (value == "1") {
    digitalWrite(GPIOPIN, LOW);
    etatGpio = "ON";
    sendToLineNotify("LED is turned ON");
  } else if (value == "0") {
    digitalWrite(GPIOPIN, HIGH);
    etatGpio = "OFF";
    sendToLineNotify("LED is turned OFF");
  }
  server.send(200, "text/html", getPage());
}

void sendToLineNotify(String message) {
  WiFiClientSecure client;
  HTTPClient http;

  client.setInsecure();
  http.begin(client, "https://notify-api.line.me/api/notify");

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Bearer " + String(LINE_NOTIFY_TOKEN));

  String payload = "message=" + message;
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println(httpCode);
    Serial.println(response);
  } else {
    Serial.printf("Error in sending message: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void setup() {
  pinMode(GPIOPIN, OUTPUT);
  digitalWrite(GPIOPIN, HIGH); // Turn off the LED initially

  Serial.begin(115200);

  // WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("WiFiNoti");

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
