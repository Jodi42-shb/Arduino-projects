#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);  // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  finger.begin(57600);  // Sensor baud rate
  mySerial.flush();

  if (finger.verifyPassword()) {
    Serial.println("Sensor found!");
  } else {
    Serial.println("Sensor not found :(");
    while (1);
  }
}

void loop() {
  Serial.println("Place finger on sensor...");

  // 1. Capture image
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    // Optional: print error codes for debugging
    return;
  }

  // 2. Convert image to template in CharBuffer 1
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return;

  // 3. Transfer template from sensor buffer to Arduino (this sends data over mySerial)
  p = finger.getModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("getModel failed");
    return;
  }

  // 4. Send header so Python knows data is coming
  Serial.print("START_BIN");

  // Now read the full response from the sensor (usually two packets)
  // getModel() already handled the command; now drain the data packets
  uint8_t templateData[534] = {0};  // Safe size
  uint16_t idx = 0;

  // Read until we get the expected amount or timeout
  unsigned long start = millis();
  while (idx < 534 && millis() - start < 2000) {
    if (mySerial.available()) {
      templateData[idx++] = mySerial.read();
    }
  }

  // Forward the raw data to PC
  for (uint16_t i = 0; i < idx; i++) {
    Serial.write(templateData[i]);
  }

  Serial.println("\nTemplate sent to PC.");
  delay(2000);  // Prevent flooding
}
