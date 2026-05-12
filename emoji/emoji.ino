#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

// --- EMOJI BITMAPS (8x12) - Removed 'const' to fix the compiler error ---

// :) Happy Face
uint8_t face_happy[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,0,0,0,0,0,0,0,0,1,0 },
  { 0,0,1,0,0,0,0,0,0,1,0,0 },
  { 0,0,0,1,1,1,1,1,1,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};

// :> Smirk
uint8_t face_smirk[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,1,1,0,0,0,0,0 },
  { 0,0,0,0,1,0,0,1,0,0,0,0 },
  { 0,0,0,1,0,0,0,0,1,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};

// :D Grin
uint8_t face_grin[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,1,1,1,1,1,1,1,1,1,0 },
  { 0,1,0,0,0,0,0,0,0,0,1,0 },
  { 0,1,1,1,1,1,1,1,1,1,1,0 },
  { 0,0,1,1,1,1,1,1,1,1,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};

// <3 Heart
uint8_t icon_heart[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,1,1,1,1,0,0,1,1,1,1,0 },
  { 0,1,1,1,1,1,1,1,1,1,1,0 },
  { 0,0,1,1,1,1,1,1,1,1,0,0 },
  { 0,0,0,1,1,1,1,1,1,0,0,0 },
  { 0,0,0,0,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,0,1,1,0,0,0,0,0 }
};

String inputBuffer = "";

void displayEmoji(String t) {
  t.trim();

  if (t == ":)") matrix.renderBitmap(face_happy, 8, 12);
  else if (t == ":>") matrix.renderBitmap(face_smirk, 8, 12);
  else if (t == ":D") matrix.renderBitmap(face_grin, 8, 12);
  else if (t == "<3") matrix.renderBitmap(icon_heart, 8, 12);
  else {
    matrix.clear();
    Serial.println("Emoji not found.");
  }
}

void setup() {
  Serial.begin(9600);
  matrix.begin();
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        displayEmoji(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }
}
