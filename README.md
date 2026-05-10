# SensorSimulator

Firmware for the LilyGO T-Display S3 that reads a small set of sensor values, encodes them into a compact binary frame, sends the frame over USB Serial, and optionally renders the latest values on the built-in TFT display.

## Project At A Glance

- Build system: PlatformIO
- Framework: Arduino for ESP32
- Main target: `lilygo-t-display-s3`
- Host test target: `native`
- Board: LilyGO T-Display S3 / ESP32-S3
- Serial speed: `115200`
- Sample and display interval: `2000 ms`

The main configuration is in `platformio.ini`.

## Repository Layout

```text
include/              Public project headers and shared types
src/                  Firmware modules
lib/test_support/     Native test mocks/stubs
test/                 PlatformIO Unity tests
platformio.ini        PlatformIO environments and build flags
```

This project keeps documentation centralized in this root README. The previous folder-level README files were PlatformIO boilerplate and did not add project-specific guidance.

## Firmware Architecture

- `src/main.cpp` coordinates the firmware loop: read sensors, encode a frame, write it to USB Serial, and refresh the display.
- `include/app_config.h` defines protocol bytes, feature flags, and timing constants.
- `include/types.h` defines `SensorData_t`, `SensorBatch_t`, `SensorId`, and `UnitId`.
- `src/hal.cpp` contains the hardware abstraction for internal temperature and ADC reads.
- `src/sensors.cpp` converts HAL readings into typed sensor records.
- `src/protocol.cpp` encodes sensor batches into the binary wire format.
- `src/display.cpp` initializes and updates the LilyGO TFT display when `USE_DISPLAY` is enabled.

## Sensor Values

The firmware currently emits four records per sample:

| Sensor | ID | Unit | Source |
| --- | ---: | --- | --- |
| Internal temperature | `0` | Celsius | `temperatureRead()` |
| Voltage | `1` | Volt | ADC channel `1` |
| Current | `2` | Amp | ADC channel `2`, converted with 2.5 V offset and 0.1 V/A sensitivity |
| External temperature | `3` | Celsius | ADC channel `3`, converted as `voltage * 100` |

The board ADC pin mapping lives in `include/hal_board.h`.

## Binary Serial Protocol

The firmware writes binary frames to USB Serial. A PC receiver should parse bytes as a stream, not as text.

```text
START(0xAA) | COUNT(1) | SENSOR_RECORD x COUNT | CHECKSUM(1) | END(0x55)
```

Each `SENSOR_RECORD` is 10 bytes:

```text
sensor_id(1) | timestamp_ms(4, little-endian) | unit_id(1) | value_float(4, little-endian IEEE-754)
```

Total frame length is:

```text
4 + COUNT * 10
```

The checksum is an 8-bit wrapping sum of `START_BYTE`, `COUNT`, and all sensor record bytes. It excludes the checksum byte itself and `END_BYTE`.

Recommended receiver behavior:

- Buffer incoming serial bytes.
- Resynchronize on `0xAA`.
- Use `COUNT` to calculate the expected frame size.
- Verify `END_BYTE == 0x55`.
- Recalculate and compare the checksum.
- If validation fails, discard bytes until the next `0xAA`.

## Display Configuration

Display support is enabled by default through `USE_DISPLAY` in `include/app_config.h`.

The PlatformIO board environment uses `TFT_eSPI` with the LilyGO T-Display S3 setup:

```ini
-D USER_SETUP_LOADED=1
-include $PROJECT_LIBDEPS_DIR/$PIOENV/TFT_eSPI/User_Setups/Setup206_LilyGo_T_Display_S3.h
```

To compile without display support, add or override this build flag:

```ini
-D USE_DISPLAY=0
```

## Build, Upload, And Monitor

Install PlatformIO Core or use the PlatformIO extension in VS Code.

Build the firmware:

```powershell
pio run -e lilygo-t-display-s3
```

Upload to the board:

```powershell
pio run -e lilygo-t-display-s3 -t upload
```

Open the serial monitor:

```powershell
pio device monitor -b 115200
```

Because the stream is binary, the monitor may show unreadable characters. Use a receiver/parser when validating protocol frames.

## Tests

The test suite uses PlatformIO's Unity runner with the `native` environment.

Run all host tests:

```powershell
pio test -e native
```

Run one test suite:

```powershell
pio test -e native -f test_protocol
pio test -e native -f test_sensors
```

The native environment is configured with:

- `test_build_src = true`, so production modules are compiled into tests.
- `UNIT_TEST`, so sensor code uses the Arduino test shim instead of the board Arduino headers.
- `build_src_filter`, which excludes Arduino-only firmware entrypoints and drivers: `main.cpp`, `display.cpp`, and `hal.cpp`.
- `lib/test_support`, which provides mocks for `millis()`, `HW_Read_InternalTemp()`, and `HW_Read_ADC()`.

Current coverage:

- `test/test_protocol` checks frame structure, checksum calculation, and little-endian float encoding.
- `test/test_sensors` checks sensor IDs, timestamps, units, and conversion formulas using mocked HAL inputs.

## Adding Tests

Add new Unity tests under `test/<suite_name>/test_<suite_name>.cpp`.

For logic that can run on the host, keep hardware calls behind small abstractions and provide mock implementations in `lib/test_support`. If a module depends directly on Arduino-only libraries or board peripherals, either isolate that dependency or exclude it from the `native` environment and test the pure logic around it.
