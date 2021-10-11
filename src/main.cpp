/* ------------------------------------------------- */

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
//#include <time.h>
//#include <PageBuilder.h>
#include <AutoConnect.h>

#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif

AutoConnect Portal(Server);
//AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4
//AutoConnectAux    Timezone;

#include "ESPTelnet.h"

/* ------------------------------------------------- */

#define SERIAL_SPEED  115200
#define WIFI_SSID     "MY SSID"
#define WIFI_PASSWORD "MY PASS"

/* ------------------------------------------------- */

ESPTelnet telnet;
IPAddress ip;

/* ------------------------------------------------- */

void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}

void setupSerial(long speed, String msg = "") {
  Serial.begin(speed);
  while (!Serial) {
  }
  delay(200);
  Serial.println();
  Serial.println();
  if (msg != "") Serial.println(msg);
}

/* ------------------------------------------------- */

bool isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

/* ------------------------------------------------- */

bool connectToWiFi(const char* ssid, const char* password, int max_tries = 20, int pause = 500) {
  int i = 0;
  WiFi.mode(WIFI_STA);
#if defined(ARDUINO_ARCH_ESP8266)
  WiFi.forceSleepWake();
  delay(200);
#endif
  WiFi.begin(ssid, password);
  do {
    delay(pause);
    Serial.print(".");
  } while (!isConnected() || i++ < max_tries);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  return isConnected();
}

/* ------------------------------------------------- */

void errorMsg(String error, bool restart = true) {
  Serial.println(error);
  if (restart) {
    Serial.println("Rebooting now...");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
}

/* ------------------------------------------------- */

// (optional) callback functions for telnet events
void onTelnetConnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" connected");
  telnet.println("\nWelcome " + telnet.getIP());
  telnet.println("(Use ^] + q  to disconnect.)");
}

void onTelnetDisconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" disconnected");
}

void onTelnetReconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" reconnected");
}

void onTelnetConnectionAttempt(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" tried to connected");
}

/* ------------------------------------------------- */

void setupTelnet() {
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);

  // passing a lambda function
  telnet.onInputReceived([](String str) {
    // checks for a certain command
    if (str == "ping") {
      telnet.println("> pong");
      Serial.println("- Telnet: pong");
    }
  });

  Serial.print("- Telnet: ");
  if (telnet.begin()) {
    Serial.println("running");
  } else {
    Serial.println("error.");
    errorMsg("Will reboot...");
  }
}

/* ------------------------------------------------- */

void setup() {
  setupSerial(SERIAL_SPEED, "Telnet Test");

  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    setupTelnet();
  } else {
    Serial.println();
    errorMsg("Error connecting to WiFi");
  }

  //Serial.print("- Wifi: ");
  //connectToWiFi(WIFI_SSID, WIFI_PASSWORD);
  // if (isConnected()) {
  //   ip = WiFi.localIP();
  //   Serial.print(" ");
  //   Serial.println(ip);
  //   setupTelnet();
  // } else {
  //   Serial.println();
  //   errorMsg("Error connecting to WiFi");
  // }
}

/* ------------------------------------------------- */

void loop() {
  Portal.handleClient();
  telnet.loop();

  // send serial input to telnet as output
  if (Serial.available()) {
    telnet.print(Serial.read());
  }
}
//* ------------------------------------------------- */
