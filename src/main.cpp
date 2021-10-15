/* ------------------------------------------------- */

#include <Arduino.h>
#include <AutoConnect.h>
#include <ESPTelnet.h>
#include <WebServer.h>
#include <WiFi.h>

#define SERIAL_SPEED 115200

WebServer   server;
AutoConnect portal(server);
ESPTelnet   telnet;
IPAddress   ip;

/* ------------------------------------------------- */

void rootPage() {
  char content[] = "Welcom to ATOM Lite";
  server.send(200, "text/plain", content);
}

void setupSerial1(long speed, String msg = "") {
  Serial.begin(speed, SERIAL_8N1);
  Serial1.begin(speed, SERIAL_8N1, 32, 26);

  Serial.flush();
  Serial1.flush();

  delay(200);
  Serial.println();
  Serial.println();
  if (msg != "") Serial.println(msg);
}

/* ------------------------------------------------- */

bool isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

void errorMsg(String error, bool restart = true) {
  Serial1.println(error);
  if (restart) {
    Serial1.println("Rebooting now...");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
}

/* ------------------------------------------------- */

// (optional) callback functions for telnet events
void onTelnetConnect(String ip) {
  Serial1.print("- Telnet: ");
  Serial1.print(ip);
  Serial1.println(" connected");
  telnet.println("\nWelcome " + telnet.getIP());
  telnet.println("(Use ^] + q  to disconnect.)");
}

void onTelnetDisconnect(String ip) {
  Serial1.print("- Telnet: ");
  Serial1.print(ip);
  Serial1.println(" disconnected");
}

void onTelnetReconnect(String ip) {
  Serial1.print("- Telnet: ");
  Serial1.print(ip);
  Serial1.println(" reconnected");
}

void onTelnetConnectionAttempt(String ip) {
  Serial1.print("- Telnet: ");
  Serial1.print(ip);
  Serial1.println(" tried to connected");
}

/* ------------------------------------------------- */

void setupTelnet() {
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);

  Serial1.print("- Telnet: ");
  if (telnet.begin()) {
    Serial1.println("running");

  } else {
    Serial1.println("error.");
    errorMsg("Will reboot...");
  }
}

/* ------------------------------------------------- */

void setup() {
  setupSerial1(SERIAL_SPEED, "Telnet Test");

  server.on("/", rootPage);
  if (portal.begin()) {
    ip = WiFi.localIP();
    Serial1.println("WiFi connected: " + WiFi.localIP().toString());
    setupTelnet();
  } else {
    Serial1.println();
    errorMsg("Error connecting to WiFi");
  }
}

/* ------------------------------------------------- */

void loop() {
  portal.handleClient();
  telnet.loop();

  if (Serial1.available() > 0) {
    telnet.println(Serial1.readString());
  }

  if (telnet.available() > 0) {
#if defined(DEBUG)
    String input(telnet.readString());
    Serial1.print(input);
    Serial.print(input);
#else
    Serial1.print(telnet.readString());
#endif
  }
}
//* ------------------------------------------------- */
