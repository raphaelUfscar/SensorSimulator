#include <Arduino.h>
#include <stdint.h>

#include "hal.h"
#include "hal_board.h"

// ADC Pins
static constexpr uint8_t ADC_PIN_VOLTAGE = HAL_GPIO_VOLTAGE;
static constexpr uint8_t ADC_PIN_CURRENT = HAL_GPIO_CURRENT;
static constexpr uint8_t ADC_PIN_EXTERNAL_TEMP = HAL_GPIO_EXT_TEMP;

float HW_Read_InternalTemp(void) {
    return temperatureRead();  // Celsius
}

float HW_Read_ADC(uint8_t channel) {
    int pin;
    switch (channel) {
        case 1: pin = ADC_PIN_VOLTAGE; break;
        case 2: pin = ADC_PIN_CURRENT; break;
        case 3: pin = ADC_PIN_EXTERNAL_TEMP; break;
        default: return 0.0f;
    }
    int raw = analogRead(pin);
    return (raw / 4095.0f) * 3.3f;
}

