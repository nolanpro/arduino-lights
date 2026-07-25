// To use ArduinoGraphics APIs, include BEFORE Arduino_LED_Matrix
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

const char name[] = "Nolan";

void setup() {
  matrix.begin();
}

void loop() {
  for (int i = 0; name[i] != '\0'; i++) {
    char letter[2] = {name[i], '\0'};

    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textFont(Font_5x7);
    matrix.text(letter, 3, 1);
    matrix.endDraw();

    delay(1000);
  }
}
