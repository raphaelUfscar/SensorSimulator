#pragma once

#include <stdint.h>
#include "app_config.h"

// Sensor IDs
enum SensorId : uint8_t {
    SENSOR_INTERNAL_TEMP = 0,
    SENSOR_VOLTAGE = 1,
    SENSOR_CURRENT = 2,
    SENSOR_EXTERNAL_TEMP = 3,
};

// Unit IDs
enum UnitId : uint8_t {
    UNIT_CELSIUS = 0,
    UNIT_VOLT = 1,
    UNIT_AMP = 2,
};

typedef struct
{
    uint8_t sensor_id;
    uint32_t timestamp;
    uint8_t unit_id;
    float value;
} SensorData_t;

typedef struct
{
    uint8_t count;
    SensorData_t sensors[MAX_SENSORS];
} SensorBatch_t;

