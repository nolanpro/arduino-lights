#ifndef STRIP_API_H
#define STRIP_API_H

#include <stdint.h>

// Minimal API declarations for the strip controller.
// This header avoids pulling in FastLED.h so it can coexist
// with Arduino_LED_Matrix.h in the same compilation unit.

void strip_setup();
void strip_loop();
void strip_on();
void strip_off();
void strip_toggle();
bool is_strip_on();
void strip_set_scene(const char* name);
const char* strip_get_scene();
void strip_set_single_led_pixel(int index);
void strip_set_hue(uint8_t hue);
uint8_t strip_get_hue();
void strip_set_color(uint8_t r, uint8_t g, uint8_t b);
void strip_get_color(uint8_t* r, uint8_t* g, uint8_t* b);
void strip_set_brightness(uint8_t brightness);
uint8_t strip_get_brightness();

#endif