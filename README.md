# ESP32 UART Bridge

A small ESP32 project for testing UART communication between two devices. The firmware can run in one of two roles:

- UART Sender (Master): sends a periodic `PING` message and listens for a response
- UART Echo (Slave): reads UART data and immediately echoes it back

This is useful for validating UART wiring, baud rate configuration, and message flow on ESP32 boards.

## Features

- ESP-IDF-based C++ project
- Selectable runtime role via menuconfig
- UART configured at 115200 baud, 8N1
- Simple echo and request-response testing
- Serial log output for received payloads

## Hardware

This project uses ESP32 UART2 on the following pins:

- RX: GPIO16
- TX: GPIO17

Connect the ESP32 UART pins to a second device or another ESP32 running the same project. For a typical loopback or two-board test, cross-connect the TX and RX lines between the devices.

## Example wiring diagram

This setup powers both boards from one USB cable connected to just one device, while the other board is powered through the shared VIN and GND connection.

![ESP32 UART wiring example](/docs/images/esp32-uart-wiring.jpg)

### Wiring description

- One USB cable is connected to only one ESP32 board
- The `VIN` line from that powered board is connected to the other board's `VIN`
- The ground lines are connected together
- Blue wire: `D16 (SENDER RX)` to `D17 (ECHO TX)`
- Yellow wire: `D17 (SENDER TX)` to `D16 (ECHO RX)`

This creates a direct UART connection between the sender and echo device while sharing the same power source.

## Runtime roles

The role is selected in the project configuration menu:

- `ROLE_SENDER` — sends a `PING` message every second and prints any reply received
- `ROLE_ECHO` — waits for incoming UART bytes and echoes them back immediately

The default role is `ROLE_ECHO`.

## Project structure

- `main/main.cpp` — UART controller and application entry point
- `main/Kconfig.projbuild` — menuconfig options for role selection
- `CMakeLists.txt` — top-level ESP-IDF project definition
- `main/CMakeLists.txt` — component build definition

## Build and flash

From the project root:

```bash
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py flash
idf.py monitor
```

When the menu opens, choose the device role under:

- `Device Role Configuration` -> `Select Device Role`

## Example behavior

### Sender mode

The sender repeatedly transmits:

```text
PING
```

and logs any bytes it receives back from the other device.

### Echo mode

The echo device reads incoming bytes and writes the same payload back to the UART immediately. This makes it easy to confirm bi-directional communication and data integrity.

## License

This project is provided as-is for experimentation and learning purposes.
