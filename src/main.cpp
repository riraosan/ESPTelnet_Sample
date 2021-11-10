#include <Arduino.h>
#include <WiFiBridge.h>
#define M5_ATOM

static WiFiBridge app;

void setup() {
#if defined(M5_ATOM)
  app.begin(115200);
#elif defined(M5_STICK)
  app.begin(115200, 33, 32, SERIAL_8N1);
#endif
}

void loop() {
  app.update();
}
