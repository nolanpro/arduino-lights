#include <FastLED.h>

constexpr int NUM_LEDS = 179;
constexpr int LED_PIN = 2;
constexpr int BRIGHTNESS = 160;

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  Serial.println("Changing to Red");
}

void loop() {
  delay(3000);

  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  Serial.println("Changing to Green");

  delay(3000);

  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  Serial.println("Changing to Blue");

  delay(3000);

  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  Serial.println("Changing to Red");

  delay(3000);

  fill_solid(leds, NUM_LEDS, CRGB(255, 160, 60));
  FastLED.show();
  Serial.println("Changing to Soft White");
}