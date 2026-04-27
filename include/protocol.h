#pragma once

#include <stddef.h>
#include <stdint.h>

#include "types.h"

void Protocol_Encode(const SensorBatch_t* batch, uint8_t* buffer, size_t* length);

