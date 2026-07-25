// Include matrix before FastLED — FastLED defines a PORT macro that
// breaks R_PFS->PORT in Arduino_LED_Matrix.h
#include "Arduino_LED_Matrix.h"
#include <FastLED.h>

#define LED_PIN     2
#define NUM_LEDS    500  // overestimate until exact count is known
#define BRIGHTNESS  100   // keep low until power wiring is confirmed

#define FRAME_MS    20
#define HUE_STEP    1    // increase for a faster rainbow cycle

#define MATRIX_COLS 12

CRGB leds[NUM_LEDS];
ArduinoLEDMatrix matrix;
uint8_t frame[8][MATRIX_COLS] = {};
uint8_t hue = 0;
uint8_t lastLit = 255;  // force first draw

void updateProgressBar(uint8_t h) {
  // Map hue 0–255 onto 0–12 lit LEDs on the top row; wraps clear the bar.
  uint8_t lit = (uint16_t)h * MATRIX_COLS / 256;
  if (lit == lastLit) {
    return;
  }
  lastLit = lit;

  memset(frame, 0, sizeof(frame));
  for (uint8_t x = 0; x < lit; x++) {
    frame[0][x] = 1;
  }
  matrix.renderBitmap(frame, 8, MATRIX_COLS);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("StripLightsTest — WS2811 solid rainbow fade"));

  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  matrix.begin();
  updateProgressBar(hue);

  Serial.print(F("LEDs: "));
  Serial.println(NUM_LEDS);
}

void loop() {
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
  FastLED.show();
  updateProgressBar(hue);
  hue += HUE_STEP;
  delay(FRAME_MS);
}
