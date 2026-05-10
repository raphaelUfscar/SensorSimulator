#pragma once

#include <stddef.h>
#include <stdint.h>

#include "types.h"

bool Protocol_Encode(const SensorBatch_t* batch, uint8_t* buffer, size_t buffer_capacity, size_t* length);
void Protocol_Encode(const SensorBatch_t* batch, uint8_t* buffer, size_t* length);

