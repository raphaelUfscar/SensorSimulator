#include <stdint.h>

#include "mocks.h"

static unsigned long g_mock_millis = 0;
static float g_mock_internal_temp = 0.0f;
static float g_mock_adc[256] = {0};

unsigned long millis() {
    return g_mock_millis;
}

void set_mock_millis(unsigned long value) {
    g_mock_millis = value;
}

void set_mock_internal_temp(float value) {
    g_mock_internal_temp = value;
}

float HW_Read_InternalTemp() {
    return g_mock_internal_temp;
}

void set_mock_adc(uint8_t channel, float value) {
    g_mock_adc[channel] = value;
}

float HW_Read_ADC(uint8_t channel) {
    return g_mock_adc[channel];
}

