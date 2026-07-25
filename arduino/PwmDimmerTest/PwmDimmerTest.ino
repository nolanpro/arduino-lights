#define PWM_PIN     6

#define FADE_STEP   2     // brightness change per frame (1 = slowest, higher = faster)
#define HOLD_MS     1500  // pause at full on / full off
#define FRAME_MS    15    // ~3s fade at step 2

uint8_t brightness = 0;
int8_t fadeDir = 1;
uint32_t holdStartMs = 0;
bool holding = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("PwmDimmerTest — strip on pin 6"));

  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, 0);
}

void loop() {
  if (holding) {
    if (millis() - holdStartMs >= HOLD_MS) {
      holding = false;
      fadeDir = -fadeDir;
    }
    delay(FRAME_MS);
    return;
  }

  brightness = constrain(brightness + fadeDir * FADE_STEP, 0, 255);
  analogWrite(PWM_PIN, brightness);

  if (brightness == 0 || brightness == 255) {
    holding = true;
    holdStartMs = millis();
  }

  delay(FRAME_MS);
}
