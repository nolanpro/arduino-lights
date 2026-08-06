#include "scenes.h"

CRGB leds[NUM_LEDS];

// ── Single LED ───────────────────────────────────────────────────────────────
// Turns on exactly one LED at full white. Pixel index set externally.
int singleLedPixelIndex = 0;

void scene_single_led() {
  FastLED.clear();
  leds[singleLedPixelIndex] = CRGB::White;
}

// ── Solid color ──────────────────────────────────────────────────────────────
// Fills all LEDs with a single hue at full saturation and value.
// Hue is set externally via solidColorHue (0-359), or full RGB via solidColorR/G/B.
int solidColorHue = 0;
int solidColorR = 255;
int solidColorG = 255;
int solidColorB = 255;

void scene_solid_color() {
  fill_solid(leds, NUM_LEDS, CRGB(solidColorR, solidColorG, solidColorB));
}

// ── Blue noise ──────────────────────────────────────────────────────────────
// Persistent palette state for the blue-noise scene.
static CRGBPalette16 currentPalette(CRGB::Black);
static CRGBPalette16 targetPalette(OceanColors_p);

void scene_blue_noise() {
  const uint8_t scale = 30;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t index = inoise8(i * scale, millis() / 10 + i * scale);
    leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
  }

  EVERY_N_MILLIS(10) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 48);
  }

  EVERY_N_SECONDS(5) {
    uint8_t baseC = random8();
    targetPalette = CRGBPalette16(
      CHSV(baseC + random8(32), 255, random8(128, 255)),
      CHSV(baseC + random8(64), 255, random8(128, 255)),
      CHSV(baseC + random8(96), 192, random8(128, 255)),
      CHSV(baseC + random8(16), 255, random8(128, 255))
    );
  }
}

// ── Rainbow chase ───────────────────────────────────────────────────────────
// Full-spectrum rainbow that scrolls along the strip.
void scene_rainbow_chase() {
  uint8_t beat = (millis() / 20) & 0xFF;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t hue = (i * 256 / NUM_LEDS) + beat;
    leds[i] = CHSV(hue, 255, 255);
  }
}

void scene_solid_white() {
  fill_solid(leds, NUM_LEDS, CRGB(255, 140, 23));
}

// ── DemoReel100: Rainbow ────────────────────────────────────────────────────
// FastLED's built-in rainbow generator
uint8_t gHue = 0;

void scene_rainbow() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

// ── DemoReel100: Rainbow with Glitter ───────────────────────────────────────
// Built-in FastLED rainbow, plus some random sparkly glitter
static void addGlitter(fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void scene_rainbow_with_glitter() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
  addGlitter(80);
}

// ── DemoReel100: Confetti ──────────────────────────────────────────────────
// Random colored speckles that blink in and fade smoothly
void scene_confetti() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

// ── DemoReel100: Sinelon ───────────────────────────────────────────────────
// A colored dot sweeping back and forth, with fading trails
void scene_sinelon() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

// ── DemoReel100: Juggle ────────────────────────────────────────────────────
// Eight colored dots, weaving in and out of sync with each other
void scene_juggle() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// ── DemoReel100: BPM ───────────────────────────────────────────────────────
// Colored stripes pulsing at a defined Beats-Per-Minute (BPM)
void scene_bpm() {
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}