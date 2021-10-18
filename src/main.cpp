#include <Arduino.h>
#include <WiFiBridge.h>

static WiFiBridge app;

void setup() {
  app.setup(115200);
}

void loop() {
  app.update();
}
