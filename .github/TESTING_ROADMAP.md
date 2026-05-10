# Testing Roadmap

This document tracks test coverage progress and the remaining gaps for the PlatformIO firmware test suite.

## Implemented Native Tests

### Protocol Module (`src/protocol.cpp`)

`test/test_protocol/test_protocol.cpp` now covers:

- `Protocol_Encode()` happy-path frame shape.
- Simple one-record checksum calculation.
- Zero-record batch encoding.
- Multi-record batch ordering and checksum validation.
- Maximum supported batch size using `MAX_SENSORS`.
- Timestamp little-endian encoding.
- Float little-endian encoding.
- Negative and fractional float values.
- Checksum wraparound behavior.
- Buffer-too-small rejection through the capacity-aware overload.
- `batch.count > MAX_SENSORS` rejection.
- Null argument rejection.

The protocol API now has a safe overload:

```cpp
bool Protocol_Encode(const SensorBatch_t* batch, uint8_t* buffer, size_t buffer_capacity, size_t* length);
```

The previous firmware-friendly wrapper is still available:

```cpp
void Protocol_Encode(const SensorBatch_t* batch, uint8_t* buffer, size_t* length);
```

### Sensors Module (`src/sensors.cpp`)

`test/test_sensors/test_sensors.cpp` now covers:

- `Sensor_InternalTemp_Read()` metadata, timestamp, and mocked value passthrough.
- `Sensor_Voltage_Read()` metadata, timestamp, nominal voltage, zero voltage, and low voltage.
- `Sensor_Current_Read()` metadata, timestamp, zero current, positive current, negative current, and larger positive current conversion.
- `Sensor_ExternalTemp_Read()` metadata, timestamp, nominal temperature, zero temperature, and fractional temperature conversion.

## Remaining Coverage Gaps

The native environment intentionally excludes Arduino-only files through `build_src_filter`:

- `src/main.cpp`
- `src/display.cpp`
- `src/hal.cpp`

Those files will not show native unit-test coverage until their hardware dependencies are isolated behind mocks or they get separate hardware/integration tests.

### HAL Module (`src/hal.cpp`)

`src/hal.cpp` depends on Arduino hardware APIs such as `temperatureRead()` and `analogRead()`.

#### `HW_Read_InternalTemp()`

- **Missing function coverage**: internal temperature hardware read.
  - **Test option**: add a wrapper/mock seam for `temperatureRead()` or run a board integration test.
  - **Expected**: returned value passes through the Arduino temperature reading in Celsius.

#### `HW_Read_ADC()`

- **Missing condition**: voltage channel mapping.
  - **Test**: call `HW_Read_ADC(1)`.
  - **Expected**: reads `HAL_GPIO_VOLTAGE`.

- **Missing condition**: current channel mapping.
  - **Test**: call `HW_Read_ADC(2)`.
  - **Expected**: reads `HAL_GPIO_CURRENT`.

- **Missing condition**: external temperature channel mapping.
  - **Test**: call `HW_Read_ADC(3)`.
  - **Expected**: reads `HAL_GPIO_EXT_TEMP`.

- **Missing condition**: invalid ADC channel.
  - **Test**: call `HW_Read_ADC(0)` or another unsupported channel.
  - **Expected**: returns `0.0f` without calling `analogRead()`.

- **Missing condition**: ADC raw-to-voltage conversion.
  - **Test**: mock `analogRead()` with `0`, mid-scale, and `4095`.
  - **Expected**: returns `0.0f`, approximately mid-scale voltage, and `3.3f`.

### Display Module (`src/display.cpp`)

`src/display.cpp` depends on Arduino GPIO and `TFT_eSPI`.

#### `Display_Init()`

- **Missing function coverage**: display power and backlight initialization.
  - **Test option**: mock Arduino GPIO and `TFT_eSPI`, or validate on hardware.
  - **Expected**: configures power/backlight pins as outputs, drives both high, initializes the TFT, sets landscape rotation, clears the screen, and prints the title.

#### `Display_RenderBatch()`

- **Missing condition**: each known sensor ID.
  - **Test**: render a batch with internal temperature, voltage, current, and external temperature records.
  - **Expected**: each record is printed on the expected row with the expected label and unit.

- **Missing condition**: unknown sensor ID.
  - **Test**: render a record with an unsupported `sensor_id`.
  - **Expected**: no display line is printed for that record.

- **Missing condition**: empty batch.
  - **Test**: render `count = 0`.
  - **Expected**: no sensor lines are updated.

### Main Firmware Loop (`src/main.cpp`)

`src/main.cpp` depends on Arduino `setup()`, `loop()`, `Serial`, and timing.

- **Missing condition**: no sample before `SAMPLING_INTERVAL_MS`.
  - **Test option**: extract loop body into a testable function with mocked time and serial output.
  - **Expected**: no sensor reads, serial writes, or display updates before the interval elapses.

- **Missing condition**: sample at or after `SAMPLING_INTERVAL_MS`.
  - **Test option**: mock `millis()`, sensor reads, protocol encode, serial write, and display render.
  - **Expected**: reads all four sensors, encodes one batch, writes the encoded frame, and refreshes display when `DISPLAY_INTERVAL_MS` also elapsed.

- **Missing condition**: display disabled.
  - **Test option**: compile with `USE_DISPLAY=0`.
  - **Expected**: firmware still samples and writes serial data without calling display code.

## Suggested Next Order

1. Run `pio test -e native` in an environment with PlatformIO installed and confirm the expanded native test suite passes.
2. Run coverage again and confirm `src/protocol.cpp` and `src/sensors.cpp` improved as expected.
3. Decide whether HAL, display, and main loop should be covered with mocks or left to hardware/integration tests.
4. If native coverage for hardware-facing files is required, extract thin wrapper interfaces around Arduino GPIO, ADC, serial, display, and timing calls.
