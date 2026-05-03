# Testing Roadmap

This document tracks planned improvements for the project's test suite.

## Pending Unit Tests

### 1. Sensors Module (`src/sensors.cpp`)
- **Task**: Validate `Sensor_CreateRecord` logic.
- **Goal**: Ensure that different sensor types and unit IDs are correctly mapped into the `SensorData_t` structure.

### 2. Protocol Module (`src/protocol.cpp`)
- **Task**: Buffer Overflow Protection.
- **Goal**: Provide a batch larger than the target buffer to `Protocol_Encode` and verify it handles length limits safely.
- **Task**: Multi-record Checksum Verification.
- **Goal**: Ensure the 8-bit sum remains accurate across variable batch sizes.