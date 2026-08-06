#include "strip_controller.h"
#include "web_server.h"

// A decent soft white light is 255,140,23

void setup() {
  strip_setup();
  web_setup();

  // Power-on default: on, full brightness, rainbow chase
  // strip_set_scene("solid-white");
  strip_set_scene("rainbow-chase");
  strip_on();
}

void loop() {
  strip_loop();
  web_loop();
}