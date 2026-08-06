#ifndef SCENES_H
#define SCENES_H

#include <FastLED.h>

constexpr int NUM_LEDS = 179;
constexpr int LED_PIN = 2;
constexpr int BRIGHTNESS = 200;

extern CRGB leds[NUM_LEDS];

// Shared pixel index for the single-LED scene.
// Set via API before activating the scene.
extern int singleLedPixelIndex;

// Global hue counter for DemoReel100 animated scenes.
// Incremented periodically in strip_loop().
extern uint8_t gHue;

// Hue value (0-359) for the solid-color scene.
// Set via API before/during the scene.
extern int solidColorHue;

// RGB color for the solid-color scene (used when set directly).
extern int solidColorR;
extern int solidColorG;
extern int solidColorB;

void scene_blue_noise();
void scene_rainbow_chase();
void scene_solid_white();
void scene_single_led();
void scene_solid_color();

// DemoReel100 scenes
void scene_rainbow();
void scene_rainbow_with_glitter();
void scene_confetti();
void scene_sinelon();
void scene_juggle();
void scene_bpm();

#endif