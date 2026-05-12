import serial
import time
import os

port = '/dev/ttyUSB2'   # Change if needed (check with `ls /dev/ttyUSB*`)
baud = 9600

try:
    ser = serial.Serial(port, baud, timeout=2)
    print(f"Connected to {port}. Place finger on sensor...")

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if "START_BIN" in line or ser.read_until(b"START_BIN"):
                print("Header detected! Receiving binary template...")

                # Read the expected template size (adjust if you see different length)
                raw_template = ser.read(534)   # Common size including headers

                if len(raw_template) >= 512:
                    filename = f"template_{int(time.time())}.bin"
                    with open(filename, "wb") as f:
                        f.write(raw_template)
                    print(f"Saved reusable template: {filename} ({len(raw_template)} bytes)")
                    print("You can now use this .bin file to load back into a sensor later.")
                else:
                    print(f"Warning: Received only {len(raw_template)} bytes. Try again.")

except KeyboardInterrupt:
    print("\nClosing serial port...")
    ser.close()
except Exception as e:
    print(f"Error: {e}")
