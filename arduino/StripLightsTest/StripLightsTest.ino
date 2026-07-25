#include <FastLED.h>

#define LED_PIN     2
#define NUM_LEDS    500  // overestimate until exact count is known
#define BRIGHTNESS  100  // keep low until power wiring is confirmed

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("StripLightsTest — solid green"));

  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();

  Serial.print(F("LEDs: "));
  Serial.println(NUM_LEDS);
}

void loop() {
  // solid green — nothing to update
}
