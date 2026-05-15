#include <ArduinoBLE.h>

// Define a custom UUID for the Service and Characteristic
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // Built-in LED on UNO R4

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // Set BLE name and advertised service
  BLE.setLocalName("ArduinoR4");
  BLE.setAdvertisedService(ledService);

  // Add characteristic to service and add service to BLE
  ledService.addCharacteristic(switchCharacteristic);
  BLE.addService(ledService);

  // Start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // Listen for BLE centrals (phones) connecting
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      // If the central app writes a value to the characteristic
      if (switchCharacteristic.written()) {
        byte value = switchCharacteristic.value();

        if (value) {
          Serial.println("LED ON");
          digitalWrite(ledPin, HIGH);
        } else {
          Serial.println("LED OFF");
          digitalWrite(ledPin, LOW);
        }
      }
    }
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
