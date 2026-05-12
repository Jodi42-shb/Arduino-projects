#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

uint8_t frame[8][12];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  matrix.begin();

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 12; col++) {
      frame[row][col] = 1;
    }
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  matrix.renderBitmap(frame, 8, 12);

  delay(1000);

  digitalWrite(LED_BUILTIN, LOW);
  matrix.clear();

  delay(1000);
}
