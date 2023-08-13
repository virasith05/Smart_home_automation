#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Servo.h>

// DHT sensor settings
#define DHTPIN D8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// IR sensor and LED settings
const int irPin = D1;
const int ledPin = D7;
const int buzzerPin = D5; // Buzzer pin

// Servo settings
Servo myservo;
const int servoPin = D0;
int servoPosition = 0;

/* Put your SSID & Password */
const char *ssid = "esp";  // Enter SSID here
const char *password = "";  // Enter Password here

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Web server setup
ESP8266WebServer server(80);

void setupWiFi();
void handleRoot();
void handleOpen();
void handleClose();
void handle_NotFound();

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(irPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT); // Initialize the buzzer pin
  myservo.attach(servoPin);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  // Web server handlers
  server.on("/", HTTP_GET, handleRoot);
  server.on("/open", HTTP_GET, handleOpen);
  server.on("/close", HTTP_GET, handleClose);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Read DHT temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Read IR sensor
  int irValue = digitalRead(irPin);
  
  // Update LED based on IR sensor
  if (irValue == LOW) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  // Control servo position
  myservo.write(servoPosition);

  // Control the buzzer based on temperature
  if (temperature > 34.0) {
    digitalWrite(buzzerPin, HIGH);
    if (servoPosition == 0){
      handleOpen();
    }
  } else {
    digitalWrite(buzzerPin, LOW);
  }
}

void handleRoot() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  String html = "<html><head><style>";
  html += "body{font-family: Arial, sans-serif; text-align: center;}";
  html += "h1{color: #3498db;}";
  html += ".container{margin-top: 20px;}";
  html += ".btn{display: inline-block; margin: 10px; padding: 10px 20px; background-color: #3498db; color: #fff; text-decoration: none; border-radius: 5px;}";
  html += ".sensor-info{margin-top: 20px; background-color: #f1f1f1; padding: 10px; border-radius: 5px;}";
  html += ".alert{color: red; font-weight: bold;}";
  html += "</style></head><body>";
  html += "<h1>Smart Home Control</h1>";
  html += "<div class='container'>";
  html += "<a class='btn' href='/open'>Open Servo</a>";
  html += "<a class='btn' href='/close'>Close Servo</a>";
  html += "</div>";
  html += "<div class='sensor-info'>";
  html += "<p>Temperature: " + String(temperature) + " &#8451;</p>";
  html += "<p>Humidity: " + String(humidity) + " %</p>";
  html += "<p>IR Sensor: " + String(digitalRead(irPin) == HIGH ? "NO OBJECT" : "Object Detected") + "</p>";
  html += "<p>LED: " + String(digitalRead(ledPin) == HIGH ? "On" : "Off") + "</p>";
  html += "<p>Buzzer: " + String(digitalRead(buzzerPin) == HIGH ? "On" : "Off") + "</p>";
  html += "<p>Servo Position: " + String(servoPosition) + "</p>";
  html += "<h3 style='color: red; font-weight: bold;'>Buzzer: " + String(digitalRead(buzzerPin) == HIGH ? "EMERGENCY!...TEMP HIGH...GET OUT" : "") + "</h3>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleOpen() {
  servoPosition = 180;
  server.send(200, "text/plain", "Servo opened");
}

void handleClose() {
  servoPosition = 0;
  server.send(200, "text/plain", "Servo closed");
}

void handle_NotFound() {
  String html = "<html><body>";
  html += "<h1>404 - Not Found</h1>";
  html += "<p>The requested resource was not found on this server.</p>";
  html += "</body></html>";
  server.send(404, "text/html", html);
}
