#include "strip_api.h"
#include "scenes.h"

static bool stripPower = false;
static char currentScene[32] = "blue-noise";
static int stripBrightness = BRIGHTNESS;
static int stripHue = 0;

void strip_setup() {
  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
  // FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(stripBrightness);
  FastLED.clear();
  FastLED.show();
}

void strip_on() {
  stripPower = true;
  FastLED.setBrightness(stripBrightness);
}

void strip_off() {
  stripPower = false;
  FastLED.clear();
  FastLED.show();
}

void strip_toggle() {
  if (stripPower) {
    strip_off();
  } else {
    strip_on();
  }
}

bool is_strip_on() {
  return stripPower;
}

void strip_set_scene(const char* name) {
  strncpy(currentScene, name, sizeof(currentScene) - 1);
  currentScene[sizeof(currentScene) - 1] = '\0';
  // Sync hue and RGB values when switching to solid-color
  if (strcmp(name, "solid-color") == 0) {
    solidColorHue = stripHue;
    CHSV hsv(stripHue, 255, 255);
    CRGB rgb = hsv;
    solidColorR = rgb.r;
    solidColorG = rgb.g;
    solidColorB = rgb.b;
  }
}

const char* strip_get_scene() {
  return currentScene;
}

void strip_loop() {
  if (!stripPower) {
    return;
  }

  if (strcmp(currentScene, "blue-noise") == 0) {
    scene_blue_noise();
  } else if (strcmp(currentScene, "rainbow-chase") == 0) {
    scene_rainbow_chase();
  } else if (strcmp(currentScene, "solid-white") == 0) {
    scene_solid_white();
  } else if (strcmp(currentScene, "single-led") == 0) {
    scene_single_led();
  } else if (strcmp(currentScene, "solid-color") == 0) {
    scene_solid_color();
  } else if (strcmp(currentScene, "rainbow") == 0) {
    scene_rainbow();
  } else if (strcmp(currentScene, "rainbow-with-glitter") == 0) {
    scene_rainbow_with_glitter();
  } else if (strcmp(currentScene, "confetti") == 0) {
    scene_confetti();
  } else if (strcmp(currentScene, "sinelon") == 0) {
    scene_sinelon();
  } else if (strcmp(currentScene, "juggle") == 0) {
    scene_juggle();
  } else if (strcmp(currentScene, "bpm") == 0) {
    scene_bpm();
  }

  // Slowly cycle the base color through the rainbow for DemoReel100 scenes
  EVERY_N_MILLISECONDS(20) { gHue++; }

  // noInterrupts();
  FastLED.show();
  // interrupts();
  // FastLED.delay(16);
}

void strip_set_single_led_pixel(int index) {
  singleLedPixelIndex = index;
}

void strip_set_hue(uint8_t hue) {
  stripHue = hue;
  solidColorHue = hue;
  // Convert hue to RGB for the color endpoint
  CHSV hsv(hue, 255, 255);
  CRGB rgb = hsv;
  solidColorR = rgb.r;
  solidColorG = rgb.g;
  solidColorB = rgb.b;
}

uint8_t strip_get_hue() {
  return stripHue;
}

void strip_set_color(uint8_t r, uint8_t g, uint8_t b) {
  solidColorR = r;
  solidColorG = g;
  solidColorB = b;
  // Derive hue from RGB for the hue endpoint
  CRGB rgb(r, g, b);
  CHSV hsv = rgb2hsv_approximate(rgb);
  stripHue = hsv.hue;
  solidColorHue = hsv.hue;
}

void strip_get_color(uint8_t* r, uint8_t* g, uint8_t* b) {
  *r = solidColorR;
  *g = solidColorG;
  *b = solidColorB;
}

void strip_set_brightness(uint8_t brightness) {
  stripBrightness = brightness;
  FastLED.setBrightness(stripBrightness);
}

uint8_t strip_get_brightness() {
  return stripBrightness;
}