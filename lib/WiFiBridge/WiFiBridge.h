
#pragma once

#include <Arduino.h>
#include <AutoConnect.h>
#include <ESPTelnet.h>
#include <WebServer.h>

class WiFiBridge {
 public:
  WiFiBridge() : _portal(_server) {
  }

  ~WiFiBridge() {}

  static void rootPage() {
    char content[] = "Welcom to ATOM Lite";
    _server.send(200, "text/plain", content);
  }

  void setupSerial1(long speed, uint32_t config, int8_t rxPin, int8_t txPin, String msg = "") {
    Serial.begin(speed, config);
    Serial1.begin(speed, config, rxPin, txPin);

    Serial.flush();
    Serial1.flush();

    delay(200);
    Serial.println();
    Serial.println();
    if (msg != "") Serial.println(msg);
  }

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

  static void onTelnetConnect(String ip) {
    Serial1.print("- Telnet: ");
    Serial1.print(ip);
    Serial1.println(" connected");
    _telnet.println("\nWelcome " + _telnet.getIP());
    _telnet.println("(Use ^] + q  to disconnect.)");
  }

  static void onTelnetDisconnect(String ip) {
    Serial1.print("- Telnet: ");
    Serial1.print(ip);
    Serial1.println(" disconnected");
  }

  static void onTelnetReconnect(String ip) {
    Serial1.print("- Telnet: ");
    Serial1.print(ip);
    Serial1.println(" reconnected");
  }

  static void onTelnetConnectionAttempt(String ip) {
    Serial1.print("- Telnet: ");
    Serial1.print(ip);
    Serial1.println(" tried to connected");
  }

  void setupTelnet() {
    // passing on functions for various telnet events
    _telnet.onConnect(onTelnetConnect);
    _telnet.onConnectionAttempt(onTelnetConnectionAttempt);
    _telnet.onReconnect(onTelnetReconnect);
    _telnet.onDisconnect(onTelnetDisconnect);

    Serial1.print("- Telnet: ");
    if (_telnet.begin()) {
      Serial1.println("running");

    } else {
      Serial1.println("error.");
      errorMsg("Will reboot...");
    }
  }

  void setup(unsigned long speed, uint32_t config = SERIAL_8N1, int8_t rxPin = 32, int8_t txPin = 26) {
    setupSerial1(speed, config, rxPin, txPin, "Serial-Telnet Bridge via WiFi");

    _server.on("/", rootPage);
    if (_portal.begin()) {
      _ip = WiFi.localIP();
      Serial1.println("WiFi connected: " + WiFi.localIP().toString());
      setupTelnet();
    } else {
      Serial1.println();
      errorMsg("Error connecting to WiFi");
    }
  }

  void update() {
    _portal.handleClient();
    _telnet.loop();

    if (Serial1.available() > 0) {
      _telnet.print(Serial1.readString());
    }

    if (_telnet.available() > 0) {
      Serial1.print(_telnet.readString());
    }
  }

 private:
  static WebServer _server;
  static ESPTelnet _telnet;
  IPAddress        _ip;
  AutoConnect      _portal;
};

WebServer WiFiBridge::_server;
ESPTelnet WiFiBridge::_telnet;