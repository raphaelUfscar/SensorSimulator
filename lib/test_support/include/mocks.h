#pragma once

#include <stdint.h>

void set_mock_millis(unsigned long value);
void set_mock_internal_temp(float value);
void set_mock_adc(uint8_t channel, float value);

