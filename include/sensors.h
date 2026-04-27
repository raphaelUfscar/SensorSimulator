#pragma once

#include "types.h"

bool Sensor_InternalTemp_Read(SensorData_t* data);
bool Sensor_Voltage_Read(SensorData_t* data);
bool Sensor_Current_Read(SensorData_t* data);
bool Sensor_ExternalTemp_Read(SensorData_t* data);

