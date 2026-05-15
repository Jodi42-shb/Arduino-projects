#include "Arduino_LED_Matrix.h"
// Buzzer pin are 9 on digital and GND on analog


ArduinoLEDMatrix matrix;

// ── Buzzer Config ──────────────────────────────────────────────
#define BUZZER_PIN     9       // Pin 9 (matches your tested wiring)
#define BUZZER_ENABLED true    // Set to false if no buzzer is connected
#define BUZZ_FREQ      1000    // 1KHz — same as your working test
// ──────────────────────────────────────────────────────────────

// Timing constants
const int dotDelay    = 40;   // 1 unit
const int dashDelay   = 120;  // 3 units
const int symbolGap   = 40;   // 1 unit  (gap between dots/dashes in same letter)
const int letterGap   = 120;  // 3 units (gap between letters)
const int wordGap     = 280;  // 7 units (gap between words)

// A single-pixel dot at the center (Row 3, Column 5)
uint8_t dot_frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// A thin 3-pixel dash at the center (Row 3, Columns 4-6)
uint8_t dash_frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// Blank frame to turn everything off
uint8_t off_frame[8][12] = {0};

String inputBuffer = "";

// ── Buzzer helpers ─────────────────────────────────────────────
void buzzOn() {
  if (BUZZER_ENABLED) {
    tone(BUZZER_PIN, BUZZ_FREQ);
  }
}

void buzzOff() {
  if (BUZZER_ENABLED) {
    noTone(BUZZER_PIN);
  }
}
// ──────────────────────────────────────────────────────────────

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
    default:  return "";
  }
}

void showDot() {
  matrix.renderBitmap(dot_frame, 8, 12);
  buzzOn();                               // tone starts
  delay(dotDelay);
  buzzOff();                              // tone stops
  matrix.renderBitmap(off_frame, 8, 12); // LED off
}

void showDash() {
  matrix.renderBitmap(dash_frame, 8, 12);
  buzzOn();                               // tone starts
  delay(dashDelay);
  buzzOff();                              // tone stops
  matrix.renderBitmap(off_frame, 8, 12); // LED off
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
      if (code[j] == '.')      showDot();
      else if (code[j] == '-') showDash();
      delay(symbolGap);
    }

    delay(letterGap);
  }
}

void setup() {
  Serial.begin(9600);
  matrix.begin();
  pinMode(BUZZER_PIN, OUTPUT); // Always declare pin — same as your working test
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