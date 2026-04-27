# SensorSimulator (LilyGO T-Display S3)

SensorSimulator is a small **ESP32-S3** firmware project that periodically reads simulated/board-provided sensor values, **encodes them into a compact binary frame**, streams them over **USB Serial**, and (optionally) shows the latest readings on the board’s built-in TFT display.

## Framework / build system

- **Build system**: PlatformIO
- **Framework**: Arduino (ESP32 core)
- **Target board**: `lilygo-t-display-s3`
- **MCU**: Espressif **ESP32‑S3**

Project configuration lives in `SensorSimulator/platformio.ini`.

## Architecture (separation of concerns)

Code is organized into small modules so each concern is isolated and testable.

### Modules / layers

- **App orchestration**
  - `SensorSimulator/src/main.cpp`
  - Owns `setup()` / `loop()` and coordinates: read sensors → encode frame → write to USB → refresh UI

- **Configuration and shared types**
  - `SensorSimulator/include/app_config.h`: global constants (frame bytes, intervals, feature flags)
  - `SensorSimulator/include/types.h`: `SensorData_t`, `SensorBatch_t`, `SensorId`, `UnitId`

- **HAL (hardware abstraction)**
  - `SensorSimulator/include/hal_board.h`: board GPIO mapping used by HAL
  - `SensorSimulator/include/hal.h` + `SensorSimulator/src/hal.cpp`
    - `HW_Read_InternalTemp()`
    - `HW_Read_ADC(channel)`

- **Sensors (service layer)**
  - `SensorSimulator/include/sensors.h` + `SensorSimulator/src/sensors.cpp`
  - Builds `SensorData_t` records with `sensor_id`, `timestamp`, `unit_id`, `value`

- **Protocol (binary framing)**
  - `SensorSimulator/include/protocol.h` + `SensorSimulator/src/protocol.cpp`
  - `Protocol_Encode(batch, buffer, length)` produces a single binary frame suitable for a PC receiver (C#).

- **Display / UI**
  - `SensorSimulator/include/display.h` + `SensorSimulator/src/display.cpp`
  - Uses **TFT_eSPI** to show sensor values on the built-in TFT.
  - LCD power/backlight are enabled before TFT init (pins used by the LilyGO T-Display S3).

## Data protocol (USB Serial stream)

The firmware sends **binary** (not text). A receiver should treat incoming bytes as a stream and parse frames.

### Frame format

```
START(0xAA) | COUNT(1) | [SENSOR_RECORD x COUNT] | CHECKSUM(1) | END(0x55)
```

Each `SENSOR_RECORD` is 10 bytes:

```
sensor_id(1) | timestamp_ms(4, little-endian) | unit_id(1) | value_float(4, little-endian IEEE754)
```

Total frame length:

- `1 + 1 + (COUNT * 10) + 1 + 1` bytes
- i.e. **`4 + COUNT*10`**

### Checksum

Current checksum is an 8-bit sum (mod 256) of:

- `START_BYTE`, `COUNT`, and all payload bytes
- **excluding** the checksum byte itself and the `END_BYTE`

## Display configuration (T-Display S3)

The project uses the upstream `TFT_eSPI` library with the LilyGO T‑Display S3 setup file:

- `User_Setups/Setup206_LilyGo_T_Display_S3.h`

This is selected in `platformio.ini` via:

- `-D USER_SETUP_LOADED=1`
- `-include .../Setup206_LilyGo_T_Display_S3.h`

## Typical receiver behavior (PC / C#)

On the PC side, implement a small stream parser:

- Buffer bytes from the serial port
- Resync on `0xAA`
- Use `COUNT` to compute expected frame size
- Validate `END_BYTE == 0x55` and checksum
- If invalid, discard until the next `0xAA`

