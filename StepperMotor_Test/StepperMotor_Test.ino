#include <ESP8266WiFi.h> // Keep on top so the IDE uses ESP8266 libraries
#include <FS.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <AccelStepper.h>

// --------------- Pin definitions --------------- //
#define motorPin1  D1 // IN1 on the ULN2003 driver
#define motorPin2  D2 // IN2 on the ULN2003 driver
#define motorPin3  D3 // IN3 on the ULN2003 driver
#define motorPin4  D4 // IN4 on the ULN2003 driver

// ----------------------------------------- Constants and variables ----------------------------------------- //
const char * PROGMEM HOST = "esp8266_Update";
const char * PROGMEM UPDATE_PATH = "/sourceControl";
const char * PROGMEM UPDATE_UNAME = "ghost";
const char * PROGMEM UPDATE_PASS = "m%O0gsLKOkDl";
const char * PROGMEM ESP_SSID = "Test"; // ESP soft access point name | CHANGE NUMBER FOR EACH DEVICE!
const char * PROGMEM ESP_PASS = "Test1234"; // ESP soft access point password

// ---------------- Objects ---------------- //
WiFiUDP udp;
ESP8266WebServer server(80);
ESP8266WebServer updateServer(1394);
ESP8266HTTPUpdateServer httpUpdater;
AccelStepper stepperMotor(AccelStepper::HALF4WIRE , motorPin1, motorPin3, motorPin2, motorPin4); // Mind the order of the pins

void setup() {
  Serial.begin(115200);
  stepperMotor.setMaxSpeed(2000.0);
  stepperMotor.setAcceleration(200.0);
  stepperMotor.setSpeed(200);
  stepperMotor.move(8192);
}

void loop() {
  stepperMotor.run();
  Serial.println(stepperMotor.distanceToGo());
}
