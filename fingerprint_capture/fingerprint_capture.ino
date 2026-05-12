/*
  fingerprint_capture.ino
  - Waits for a command from PC over USB Serial ('c' = capture)
  - Enrolls finger into slot 1 (two scans)
  - Uploads the 512-byte template to PC as raw binary
  - RX only blinks when a command is actually received
*/

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Sensor wired to D2 (RX) and D3 (TX) on Arduino
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(57600);          // USB to PC — use 57600 to match Python
  while (!Serial);              // Wait for serial monitor on Leonardo; harmless on Uno
  finger.begin(57600);          // Sensor baud rate

  Serial.println("READY");

  if (finger.verifyPassword()) {
    Serial.println("SENSOR_OK");
  } else {
    Serial.println("SENSOR_FAIL");
    while (1) { delay(1000); }  // Halt — fix wiring before continuing
  }

  Serial.println("Send 'c' to capture fingerprint.");
}

void loop() {
  // Only act when PC sends a command — no polling the sensor
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == 'c') {
      captureAndUpload();
    }
  }
}

void captureAndUpload() {
  int p;

  // --- Scan 1 ---
  Serial.println("PLACE_FINGER");
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOFINGER)    continue;
    if (p == FINGERPRINT_PACKETRECIEVEERR) { Serial.println("ERR_COMMS");  return; }
    if (p == FINGERPRINT_IMAGEFAIL)        { Serial.println("ERR_IMAGE");  return; }
  }
  Serial.println("IMG1_OK");

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) { Serial.println("ERR_TZ1"); return; }

  // --- Lift and place again ---
  Serial.println("REMOVE_FINGER");
  delay(1500);
  while (finger.getImage() != FINGERPRINT_NOFINGER) { delay(100); }

  Serial.println("PLACE_AGAIN");
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOFINGER) continue;
  }
  Serial.println("IMG2_OK");

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) { Serial.println("ERR_TZ2"); return; }

  // --- Create model from both scans ---
  p = finger.createModel();
  if (p == FINGERPRINT_ENROLLMISMATCH) { Serial.println("ERR_MISMATCH"); return; }
  if (p != FINGERPRINT_OK)             { Serial.println("ERR_MODEL");    return; }

  // --- Store in slot 1 (needed before upload works) ---
  p = finger.storeModel(1);
  if (p != FINGERPRINT_OK) { Serial.println("ERR_STORE"); return; }

  // --- Upload template from sensor to Arduino then to PC ---
  // Adafruit library: getModel(slot) downloads template into char buffers
  // We re-load it from slot 1 into buffer 1 for upload
  p = finger.loadModel(1);
  if (p != FINGERPRINT_OK) { Serial.println("ERR_LOAD"); return; }

  // getModel() sends the UpChar command and streams 512 bytes (two 256-byte packets)
  // We capture them from the SoftwareSerial ourselves
  uint8_t packet[534];
  uint16_t idx = 0;

  // Issue the UpChar command manually for buffer 1
  // Adafruit's finger.getModel() does not expose the raw byte stream to us,
  // so we call it and read from mySerial directly afterward.
  p = finger.getModel();
  if (p != FINGERPRINT_OK) { Serial.println("ERR_GETMODEL"); return; }

  // Drain bytes from sensor into our buffer (two data packets = ~534 bytes with headers)
  unsigned long t = millis();
  while (idx < 534 && millis() - t < 3000) {
    if (mySerial.available()) {
      packet[idx++] = mySerial.read();
      t = millis();  // Reset timeout on each byte received
    }
  }

  if (idx < 512) {
    Serial.print("ERR_SHORT:");
    Serial.println(idx);
    return;
  }

  // Send structured frame to PC:
  //   4 bytes magic  : 0xDE 0xAD 0xBE 0xEF
  //   2 bytes length : little-endian uint16
  //   N bytes data
  //   1 byte checksum: XOR of all data bytes
  Serial.write(0xDE); Serial.write(0xAD);
  Serial.write(0xBE); Serial.write(0xEF);
  Serial.write((uint8_t)(idx & 0xFF));
  Serial.write((uint8_t)((idx >> 8) & 0xFF));

  uint8_t chk = 0;
  for (uint16_t i = 0; i < idx; i++) chk ^= packet[i];

  Serial.write(packet, idx);
  Serial.write(chk);
  Serial.println("DONE");
}
