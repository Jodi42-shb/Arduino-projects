#!/usr/bin/env python3
"""
fingerprint_capture.py
Communicates with fingerprint_capture.ino to save fingerprint templates to disk.

Usage:
    python3 fingerprint_capture.py                  # auto-detect port
    python3 fingerprint_capture.py /dev/ttyUSB0     # specify port
    python3 fingerprint_capture.py --load template_1234567890.bin  # verify saved file
"""

import serial
import serial.tools.list_ports
import sys
import time
import os
import struct

BAUD = 57600
MAGIC = bytes([0xDE, 0xAD, 0xBE, 0xEF])
SAVE_DIR = os.path.expanduser("~/fingerprints")


def find_arduino_port():
    """Auto-detect the Arduino serial port."""
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = (p.description or "").lower()
        mfr  = (p.manufacturer or "").lower()
        if "arduino" in desc or "ch340" in desc or "ch341" in desc \
                or "ftdi" in desc or "arduino" in mfr:
            return p.device
    # Fallback: return first available port
    if ports:
        print(f"[warn] No obvious Arduino port found. Using {ports[0].device}")
        return ports[0].device
    return None


def wait_for_line(ser, timeout=30):
    """Read lines until timeout, yielding each one."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        if ser.in_waiting:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line:
                return line
        time.sleep(0.05)
    return None


def read_until_magic(ser, timeout=60):
    """
    Scan the byte stream for the 4-byte magic header.
    Returns True when found, False on timeout.
    Also prints any text lines received while waiting.
    """
    buf = bytearray()
    deadline = time.time() + timeout
    partial_line = bytearray()

    while time.time() < deadline:
        b = ser.read(1)
        if not b:
            continue

        byte = b[0]

        # Try to print readable text lines for user feedback
        if 0x20 <= byte <= 0x7E or byte in (0x0A, 0x0D):
            partial_line.append(byte)
            if byte == 0x0A:
                text = partial_line.decode("utf-8", errors="ignore").strip()
                if text:
                    print(f"  [arduino] {text}")
                partial_line.clear()

        buf.append(byte)
        if len(buf) > 4:
            buf.pop(0)

        if bytes(buf) == MAGIC:
            return True

    return False


def receive_template(ser):
    """
    After magic is detected, read:
      2 bytes: length (little-endian)
      N bytes: template data
      1 byte:  XOR checksum
    Returns the raw template bytes or None on error.
    """
    # Read length
    length_bytes = ser.read(2)
    if len(length_bytes) < 2:
        print("[error] Timeout reading length field.")
        return None

    length = struct.unpack("<H", length_bytes)[0]
    print(f"  [info] Expecting {length} bytes of template data...")

    data = b""
    deadline = time.time() + 10
    while len(data) < length and time.time() < deadline:
        chunk = ser.read(length - len(data))
        data += chunk

    if len(data) < length:
        print(f"[error] Only received {len(data)}/{length} bytes.")
        return None

    # Read checksum
    chk_byte = ser.read(1)
    if not chk_byte:
        print("[warn] No checksum byte received.")
        return data

    expected_chk = 0
    for b in data:
        expected_chk ^= b

    if chk_byte[0] != expected_chk:
        print(f"[warn] Checksum mismatch! Got {chk_byte[0]:02X}, expected {expected_chk:02X}")
        print("       Template may be corrupted, but saving anyway.")
    else:
        print("  [info] Checksum OK ✓")

    return data


def save_template(data):
    os.makedirs(SAVE_DIR, exist_ok=True)
    filename = os.path.join(SAVE_DIR, f"template_{int(time.time())}.bin")
    with open(filename, "wb") as f:
        f.write(data)
    print(f"\n✅ Saved: {filename}  ({len(data)} bytes)")
    return filename


def inspect_template(filepath):
    """Basic inspection of a saved .bin file."""
    with open(filepath, "rb") as f:
        data = f.read()
    print(f"File: {filepath}")
    print(f"Size: {len(data)} bytes")
    print(f"First 16 bytes: {data[:16].hex(' ')}")
    print(f"Last  16 bytes: {data[-16:].hex(' ')}")


def main():
    args = sys.argv[1:]

    # --load mode: just inspect a saved file
    if args and args[0] == "--load":
        if len(args) < 2:
            print("Usage: python3 fingerprint_capture.py --load <file.bin>")
            sys.exit(1)
        inspect_template(args[1])
        return

    # Determine port
    port = args[0] if args else find_arduino_port()
    if not port:
        print("[error] No serial port found. Plug in Arduino and try again.")
        print("        Or run: python3 fingerprint_capture.py /dev/ttyUSBx")
        sys.exit(1)

    print(f"Connecting to {port} at {BAUD} baud...")

    try:
        ser = serial.Serial(port, BAUD, timeout=2)
    except serial.SerialException as e:
        print(f"[error] Could not open port: {e}")
        sys.exit(1)

    # Arduino resets on serial connect — give it time
    print("Waiting for Arduino to boot (~2s)...")
    time.sleep(2)
    ser.reset_input_buffer()

    # Wait for READY/SENSOR_OK
    print("Waiting for sensor init...")
    for _ in range(10):
        line = wait_for_line(ser, timeout=3)
        if line:
            print(f"  [arduino] {line}")
            if "SENSOR_FAIL" in line:
                print("[error] Sensor not found. Check wiring (D2→TX, D3→RX of sensor).")
                sys.exit(1)
            if "SENSOR_OK" in line:
                break

    print("\n" + "="*50)
    print("Sending capture command ('c')...")
    ser.write(b'c')

    print("\nFollow the prompts from Arduino:")
    print("  → Place your finger when told")
    print("  → Remove it when told")
    print("  → Place it again")
    print("="*50 + "\n")

    # Read status lines until binary frame arrives
    print("Waiting for template data (up to 60s)...")
    found = read_until_magic(ser, timeout=60)

    if not found:
        print("[error] Timed out waiting for template. Check Arduino Serial Monitor for errors.")
        ser.close()
        sys.exit(1)

    print("  [info] Magic header found — reading template...")
    data = receive_template(ser)

    if data:
        filepath = save_template(data)
        print(f"\nTo inspect later:\n  python3 fingerprint_capture.py --load {filepath}")
    else:
        print("[error] Failed to receive valid template.")

    # Drain any remaining output
    time.sleep(1)
    while ser.in_waiting:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print(f"  [arduino] {line}")

    ser.close()


if __name__ == "__main__":
    main()
