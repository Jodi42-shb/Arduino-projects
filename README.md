# Arduino Sketches and Fingerprint Sensor Tools

This repository is an Arduino workspace with a few small sketches for the Arduino UNO R4 LED matrix and several experiments for capturing fingerprint sensor templates/images over serial.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `UNOr4/` | Basic Arduino UNO R4 LED matrix test that fills the matrix and blinks the built-in LED. |
| `MorseCode/` | Reads text from Serial and displays Morse code dots/dashes on the UNO R4 LED matrix. |
| `emoji/` | Reads BBCode-style smileys from Serial and renders 8x12 faces on the UNO R4 LED matrix. |
| `fingerprint/` | Fingerprint capture experiments for exporting both templates and raw images, with Python conversion helpers. |
| `fingerprint_capture/` | More structured fingerprint template capture flow using a framed binary protocol and checksum. |
| `Fingerprint_logger/` | Earlier fingerprint template logger that saves binary templates from the serial stream. |

Generated captures such as `.bin`, `.raw`, and `.png` files are local artifacts and should not be committed.

## Hardware

The sketches are built around:

- Arduino UNO R4 or another compatible Arduino board.
- Arduino UNO R4 LED Matrix library for the matrix sketches.
- Adafruit-compatible optical fingerprint sensor for the fingerprint sketches.

Typical fingerprint sensor wiring:

| Sensor Pin | Arduino Pin |
| --- | --- |
| `VCC` | `5V` or `3.3V`, depending on the module |
| `GND` | `GND` |
| `TX` | Digital pin `2` |
| `RX` | Digital pin `3` |

The fingerprint sketches use `SoftwareSerial mySerial(2, 3)`, so keep this wiring unless you update the code.

## Arduino Setup

Install the required Arduino libraries through the Arduino IDE Library Manager:

- `Arduino_LED_Matrix`
- `Adafruit Fingerprint Sensor Library`

Open the `.ino` file inside the folder you want to run and upload it with the Arduino IDE. Each sketch expects to live in a folder with the same base name as the `.ino` file, which is how this repository is organized.

## LED Matrix Sketches

### `UNOr4/UNOr4.ino`

Uploads a simple test pattern:

- Turns on every LED in the 8x12 matrix.
- Blinks the built-in LED once per second.

### `MorseCode/MorseCode.ino`

Reads a line from Serial at `9600` baud and renders letters as Morse code:

- Dot: single center pixel.
- Dash: short horizontal line.
- Spaces: longer word gap.

Example Serial Monitor input:

```text
sos
```

### `emoji/emoji.ino`

Reads a smiley from Serial at `9600` baud and displays a matching 8x12 face.

Supported inputs include:

```text
:)
:|
:(
:D
:O
;)
:/
:P
:lol:
:mad:
:rolleyes:
:cool:
```

## Fingerprint Projects

### `fingerprint_capture/`

This is the cleanest template-capture flow in the repository. The Arduino waits for a `c` command, guides the user through two scans, creates a fingerprint model, stores it in slot `1`, then sends a binary frame to the Python script:

- Magic header: `DE AD BE EF`
- Two-byte little-endian length
- Template payload
- XOR checksum

Run it:

```bash
cd fingerprint_capture
uv sync
uv run python fingerprint_capture.py
```

You can also pass a serial port explicitly:

```bash
uv run python fingerprint_capture.py /dev/ttyUSB0
```

Saved templates are written to:

```text
~/fingerprints/
```

Inspect a saved template:

```bash
uv run python fingerprint_capture.py --load ~/fingerprints/template_<timestamp>.bin
```

### `Fingerprint_logger/`

An earlier logger that listens for a `START_BIN` marker and saves around `534` bytes of template data.

Before running, update the `port` variable in `Fingerprint_logger.py` if your board is not on `/dev/ttyUSB2`.

```bash
cd Fingerprint_logger
uv sync
uv run python Fingerprint_logger.py
```

### `fingerprint/`

Experimental capture utilities for templates and raw images:

- `fingerprint.ino` sends both template and image data when it receives `b`.
- `fingerprint-batch.py` captures multiple fingers into `finger_<n>/template.bin` and `finger_<n>/view.png`.
- `raw-convert.py` converts the latest `.raw` capture into a viewable PNG.

Run the batch capture:

```bash
cd fingerprint
uv sync
uv run python fingerprint-batch.py
```

Run raw conversion:

```bash
uv run python raw-convert.py
```

## Python Requirements

The Python projects use `uv` and require Python `>=3.12`.

Install `uv` if needed:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Common dependencies:

- `pyserial`
- `Pillow` for image conversion in `fingerprint/`

If you prefer plain `pip`, create a virtual environment in the project folder and install the dependencies from the corresponding `pyproject.toml`.

## Serial Port Notes

On Linux, Arduino boards usually appear as one of:

```text
/dev/ttyUSB0
/dev/ttyUSB1
/dev/ttyACM0
```

If the Python script cannot open the port, check the current device:

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

You may also need permission to access serial devices. On many Linux systems this means adding your user to the `dialout` group and logging out/in:

```bash
sudo usermod -aG dialout "$USER"
```

## Notes

- Fingerprint template data is biometric data. Treat saved `.bin`, `.raw`, and `.png` files as sensitive.
- The fingerprint sensor baud rate is usually `57600`; the USB serial baud rate varies by sketch.
- Some scripts have hard-coded ports from local testing. Update them before running on another machine.
