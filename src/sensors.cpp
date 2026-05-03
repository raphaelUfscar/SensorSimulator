#ifdef UNIT_TEST
#include "mock_arduino.h" // Include mock for native tests
#else
#include <Arduino.h>      // Original include for ESP32 build
#endif

#include "sensors.h"
#include "hal.h"

bool Sensor_InternalTemp_Read(SensorData_t* data) {
    data->sensor_id = SENSOR_INTERNAL_TEMP;
    data->timestamp = millis();
    data->unit_id = UNIT_CELSIUS;
    data->value = HW_Read_InternalTemp();
    return true;
}

bool Sensor_Voltage_Read(SensorData_t* data) {
    data->sensor_id = SENSOR_VOLTAGE;
    data->timestamp = millis();
    data->unit_id = UNIT_VOLT;
    float raw_voltage = HW_Read_ADC(1);
    data->value = raw_voltage;
    return true;
}

bool Sensor_Current_Read(SensorData_t* data) {
    data->sensor_id = SENSOR_CURRENT;
    data->timestamp = millis();
    data->unit_id = UNIT_AMP;
    float raw_voltage = HW_Read_ADC(2);
    float offset = 2.5f;
    float sensitivity = 0.1f;  // V/A
    data->value = (raw_voltage - offset) / sensitivity;
    return true;
}

bool Sensor_ExternalTemp_Read(SensorData_t* data) {
    data->sensor_id = SENSOR_EXTERNAL_TEMP;
    data->timestamp = millis();
    data->unit_id = UNIT_CELSIUS;
    float raw_voltage = HW_Read_ADC(3);
    data->value = raw_voltage * 100.0f;
    return true;
}
