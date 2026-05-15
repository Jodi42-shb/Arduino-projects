#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

// Timing constants
const int dotDelay = 25;
const int dashDelay = 70;
const int symbolGap = 20;
const int letterGap = 60;
const int wordGap = 120;

// A single-pixel dot at the center (Row 3, Column 5)
uint8_t dot_frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 }, // Row 3: Just one LED
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// A thin 6-pixel dash at the center (Row 3, Columns 3-8)
uint8_t dash_frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 }, // Row 3: Thin horizontal line
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
// Blank frame to turn everything off
uint8_t off_frame[8][12] = {0};

String inputBuffer = "";

String morseMap(char c) {
  switch (tolower(c)) {
    case 'a': return ".-";   case 'b': return "-..."; case 'c': return "-.-.";
    case 'd': return "-..";  case 'e': return ".";    case 'f': return "..-.";
    case 'g': return "--.";  case 'h': return "...."; case 'i': return "..";
    case 'j': return ".---"; case 'k': return "-.-";  case 'l': return ".-..";
    case 'm': return "--";   case 'n': return "-.";   case 'o': return "---";
    case 'p': return ".--."; case 'q': return "--.-"; case 'r': return ".-.";
    case 's': return "...";  case 't': return "-";    case 'u': return "..-";
    case 'v': return "...-"; case 'w': return ".--";  case 'x': return "-..-";
    case 'y': return "-.--"; case 'z': return "--..";
    default: return "";
  }
}

void showDot() {
  matrix.renderBitmap(dot_frame, 8, 12);
  delay(dotDelay);
  matrix.renderBitmap(off_frame, 8, 12);
}

void showDash() {
  matrix.renderBitmap(dash_frame, 8, 12);
  delay(dashDelay);
  matrix.renderBitmap(off_frame, 8, 12);
}

void sendMorse(String text) {
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    if (c == ' ') {
      delay(wordGap);
      continue;
    }
    String code = morseMap(c);
    for (int j = 0; j < code.length(); j++) {
      if (code[j] == '.') showDot();
      else if (code[j] == '-') showDash();
      delay(symbolGap);
    }
    delay(letterGap);
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
        Serial.print("Sending: ");
        Serial.println(inputBuffer);
        sendMorse(inputBuffer);
        inputBuffer = ""; 
      }
    } else {
      inputBuffer += c; 
    }
  }
}
