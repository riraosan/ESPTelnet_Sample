
#pragma once

#include <Arduino.h>
#include <AutoConnect.h>
#include <ESPTelnet.h>
#include <WebServer.h>

class WiFiBridge {
 public:
  WiFiBridge() : _portal(_server) {
  }

  void setupSerial1(long speed, uint32_t config, int8_t rxPin, int8_t txPin, String msg = "") {
    Serial1.begin(speed, config, rxPin, txPin);
    Serial1.flush();
    delay(500);
  }

  bool isConnected() {
    return (WiFi.status() == WL_CONNECTED);
  }

  void setupTelnet() {
    _telnet.onConnect([this](String ip) {
      _telnet.println("\nWelcome. Your IP address is " + ip);
    });

    _telnet.onConnectionAttempt([this](String ip) {
      Serial1.println("- Telnet: " + ip + " tried to connected");
    });

    _telnet.onReconnect([this](String ip) {
      Serial1.println("- Telnet: " + ip + " reconnected");
    });

    _telnet.onDisconnect([this](String ip) {
      Serial1.println("- Telnet: " + ip + " disconnected");
    });

    if (!_telnet.begin()) {
      ESP.restart();
    }
  }

  void begin(unsigned long speed, int8_t rxPin = 32, int8_t txPin = 26, uint32_t config = SERIAL_8N1) {
    setupSerial1(speed, config, rxPin, txPin);

    _server.on("/", [this]() {
      char content[] = "Welcome to ATOM Lite";
      _server.send(200, "text/plain", content);
    });

    if (_portal.begin()) {
      setupTelnet();
    } else {
      ESP.restart();
    }
  }

  void update() {
    _portal.handleClient();
    _telnet.loop();

    if (Serial1.available() > 0) {
      _telnet.print(Serial1.readStringUntil('\n'));
    }

    if (_telnet.available() > 0) {
      Serial1.print(_telnet.readStringUntil('\n'));
    }
  }

 private:
  WebServer   _server;
  ESPTelnet   _telnet;
  AutoConnect _portal;
};
