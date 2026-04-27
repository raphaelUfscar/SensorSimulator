#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "protocol.h"
#include "app_config.h"

void Protocol_Encode(const SensorBatch_t* batch, uint8_t* buffer, size_t* length) {
    size_t pos = 0;
    buffer[pos++] = START_BYTE;
    buffer[pos++] = batch->count;
    uint8_t checksum = static_cast<uint8_t>(START_BYTE + batch->count);

    for (uint8_t i = 0; i < batch->count; i++) {
        const SensorData_t* sensor = &batch->sensors[i];
        buffer[pos++] = sensor->sensor_id;
        checksum = static_cast<uint8_t>(checksum + sensor->sensor_id);

        // Timestamp as 4 bytes little endian
        buffer[pos++] = (sensor->timestamp >> 0) & 0xFF;
        buffer[pos++] = (sensor->timestamp >> 8) & 0xFF;
        buffer[pos++] = (sensor->timestamp >> 16) & 0xFF;
        buffer[pos++] = (sensor->timestamp >> 24) & 0xFF;
        checksum = static_cast<uint8_t>(checksum + ((sensor->timestamp >> 0) & 0xFF));
        checksum = static_cast<uint8_t>(checksum + ((sensor->timestamp >> 8) & 0xFF));
        checksum = static_cast<uint8_t>(checksum + ((sensor->timestamp >> 16) & 0xFF));
        checksum = static_cast<uint8_t>(checksum + ((sensor->timestamp >> 24) & 0xFF));

        buffer[pos++] = sensor->unit_id;
        checksum = static_cast<uint8_t>(checksum + sensor->unit_id);

        // Value as 4 bytes float little endian
        uint32_t value_bits = 0;
        static_assert(sizeof(value_bits) == sizeof(sensor->value), "float must be 32-bit");
        memcpy(&value_bits, &sensor->value, sizeof(value_bits));

        buffer[pos++] = (value_bits >> 0) & 0xFF;
        buffer[pos++] = (value_bits >> 8) & 0xFF;
        buffer[pos++] = (value_bits >> 16) & 0xFF;
        buffer[pos++] = (value_bits >> 24) & 0xFF;
        checksum = static_cast<uint8_t>(checksum + ((value_bits >> 0) & 0xFF));
        checksum = static_cast<uint8_t>(checksum + ((value_bits >> 8) & 0xFF));
        checksum = static_cast<uint8_t>(checksum + ((value_bits >> 16) & 0xFF));
        checksum = static_cast<uint8_t>(checksum + ((value_bits >> 24) & 0xFF));
    }

    buffer[pos++] = checksum;
    buffer[pos++] = END_BYTE;
    *length = pos;
}

