#pragma once

#include <stdint.h>

// Constants
constexpr uint8_t MAX_SENSORS = 4;
constexpr uint8_t START_BYTE = 0xAA;
constexpr uint8_t END_BYTE = 0x55;

// UI
#ifndef USE_DISPLAY
#define USE_DISPLAY 1
#endif

// Sampling interval (ms) - also the USB transmit rate
// 1 Hz => 1000 ms
constexpr uint32_t SAMPLING_INTERVAL_MS = 2000;

// Display refresh interval (ms)
// Keep in sync with sampling to update UI at 2 seconds as well.
constexpr uint32_t DISPLAY_INTERVAL_MS = 2000;

