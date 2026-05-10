#include <unity.h>

#include <stdint.h>
#include <string.h>

#include "mocks.h"
#include "protocol.h"
#include "types.h"
#include "app_config.h"

void setUp(void) {}
void tearDown(void) {}

static uint8_t checksum_for_encoded_frame(const uint8_t* buffer, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length - 2; i++) {
        checksum = static_cast<uint8_t>(checksum + buffer[i]);
    }
    return checksum;
}

static uint32_t float_bits(float value) {
    uint32_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

void test_protocol_frame_structure(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = 0x01;
    batch.sensors[0].timestamp = 1000; // 0x000003E8
    batch.sensors[0].unit_id = 0x01;
    batch.sensors[0].value = 25.5f;

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    TEST_ASSERT_EQUAL_UINT32(4 + (batch.count * 10), length);
    TEST_ASSERT_EQUAL_UINT8(START_BYTE, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(1, buffer[1]);
    TEST_ASSERT_EQUAL_UINT8(END_BYTE, buffer[length - 1]);
}

void test_protocol_checksum_calculation_simple(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = 0x01;
    batch.sensors[0].timestamp = 0;
    batch.sensors[0].unit_id = 0x01;
    batch.sensors[0].value = 0.0f;

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    // payload checksum is at length-2 (just before END_BYTE)
    uint8_t checksum = buffer[length - 2];

    // Checksum = START + COUNT + ID + TIME(4 bytes) + UNIT + VALUE(4 bytes)
    // => 0xAA + 0x01 + 0x01 + 0 + 0 + 0 + 0 + 0x01 + 0 + 0 + 0 + 0 = 0xAD
    TEST_ASSERT_EQUAL_UINT8(0xAD, checksum);
}

void test_protocol_little_endian_float_encoding(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = 1;
    batch.sensors[0].timestamp = 0;
    batch.sensors[0].unit_id = 1;
    batch.sensors[0].value = 10.0f; // 0x41200000 => 00 00 20 41 (LE)

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    // layout:
    // [0] START
    // [1] COUNT
    // [2] sensor_id
    // [3..6] timestamp
    // [7] unit_id
    // [8..11] float bytes
    TEST_ASSERT_EQUAL_UINT8(0x00, buffer[8]);
    TEST_ASSERT_EQUAL_UINT8(0x00, buffer[9]);
    TEST_ASSERT_EQUAL_UINT8(0x20, buffer[10]);
    TEST_ASSERT_EQUAL_UINT8(0x41, buffer[11]);

    (void)length;
}

void test_protocol_zero_record_batch(void) {
    SensorBatch_t batch{};
    batch.count = 0;

    uint8_t buffer[4]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    TEST_ASSERT_EQUAL_UINT32(4, length);
    TEST_ASSERT_EQUAL_UINT8(START_BYTE, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);
    TEST_ASSERT_EQUAL_UINT8(START_BYTE, buffer[2]);
    TEST_ASSERT_EQUAL_UINT8(END_BYTE, buffer[3]);
}

void test_protocol_multi_record_checksum_and_order(void) {
    SensorBatch_t batch{};
    batch.count = 2;
    batch.sensors[0].sensor_id = SENSOR_VOLTAGE;
    batch.sensors[0].timestamp = 0x01020304;
    batch.sensors[0].unit_id = UNIT_VOLT;
    batch.sensors[0].value = 3.3f;
    batch.sensors[1].sensor_id = SENSOR_CURRENT;
    batch.sensors[1].timestamp = 0xA0B0C0D0;
    batch.sensors[1].unit_id = UNIT_AMP;
    batch.sensors[1].value = -1.25f;

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    TEST_ASSERT_EQUAL_UINT32(24, length);
    TEST_ASSERT_EQUAL_UINT8(SENSOR_VOLTAGE, buffer[2]);
    TEST_ASSERT_EQUAL_UINT8(UNIT_VOLT, buffer[7]);
    TEST_ASSERT_EQUAL_UINT8(SENSOR_CURRENT, buffer[12]);
    TEST_ASSERT_EQUAL_UINT8(UNIT_AMP, buffer[17]);
    TEST_ASSERT_EQUAL_UINT8(checksum_for_encoded_frame(buffer, length), buffer[length - 2]);
}

void test_protocol_max_sensor_batch(void) {
    SensorBatch_t batch{};
    batch.count = MAX_SENSORS;

    for (uint8_t i = 0; i < MAX_SENSORS; i++) {
        batch.sensors[i].sensor_id = i;
        batch.sensors[i].timestamp = 1000U + i;
        batch.sensors[i].unit_id = UNIT_CELSIUS;
        batch.sensors[i].value = static_cast<float>(i) + 0.5f;
    }

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    TEST_ASSERT_EQUAL_UINT32(4 + (MAX_SENSORS * 10), length);
    TEST_ASSERT_EQUAL_UINT8(MAX_SENSORS, buffer[1]);
    TEST_ASSERT_EQUAL_UINT8(checksum_for_encoded_frame(buffer, length), buffer[length - 2]);
    TEST_ASSERT_EQUAL_UINT8(END_BYTE, buffer[length - 1]);
}

void test_protocol_timestamp_little_endian_encoding(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = SENSOR_INTERNAL_TEMP;
    batch.sensors[0].timestamp = 0x12345678;
    batch.sensors[0].unit_id = UNIT_CELSIUS;
    batch.sensors[0].value = 0.0f;

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    TEST_ASSERT_EQUAL_UINT8(0x78, buffer[3]);
    TEST_ASSERT_EQUAL_UINT8(0x56, buffer[4]);
    TEST_ASSERT_EQUAL_UINT8(0x34, buffer[5]);
    TEST_ASSERT_EQUAL_UINT8(0x12, buffer[6]);
    (void)length;
}

void test_protocol_checksum_wraparound(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = 0xFF;
    batch.sensors[0].timestamp = 0xFFFFFFFF;
    batch.sensors[0].unit_id = 0xFF;
    batch.sensors[0].value = 0.0f;

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    TEST_ASSERT_EQUAL_UINT8(checksum_for_encoded_frame(buffer, length), buffer[length - 2]);
}

void test_protocol_negative_and_fractional_float_encoding(void) {
    SensorBatch_t batch{};
    batch.count = 2;
    batch.sensors[0].sensor_id = SENSOR_CURRENT;
    batch.sensors[0].timestamp = 0;
    batch.sensors[0].unit_id = UNIT_AMP;
    batch.sensors[0].value = -1.25f;
    batch.sensors[1].sensor_id = SENSOR_EXTERNAL_TEMP;
    batch.sensors[1].timestamp = 0;
    batch.sensors[1].unit_id = UNIT_CELSIUS;
    batch.sensors[1].value = 0.125f;

    uint8_t buffer[64]{};
    size_t length = 0;
    TEST_ASSERT_TRUE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));

    const uint32_t negative_bits = float_bits(-1.25f);
    TEST_ASSERT_EQUAL_UINT8((negative_bits >> 0) & 0xFF, buffer[8]);
    TEST_ASSERT_EQUAL_UINT8((negative_bits >> 8) & 0xFF, buffer[9]);
    TEST_ASSERT_EQUAL_UINT8((negative_bits >> 16) & 0xFF, buffer[10]);
    TEST_ASSERT_EQUAL_UINT8((negative_bits >> 24) & 0xFF, buffer[11]);

    const uint32_t fractional_bits = float_bits(0.125f);
    TEST_ASSERT_EQUAL_UINT8((fractional_bits >> 0) & 0xFF, buffer[18]);
    TEST_ASSERT_EQUAL_UINT8((fractional_bits >> 8) & 0xFF, buffer[19]);
    TEST_ASSERT_EQUAL_UINT8((fractional_bits >> 16) & 0xFF, buffer[20]);
    TEST_ASSERT_EQUAL_UINT8((fractional_bits >> 24) & 0xFF, buffer[21]);
}

void test_protocol_rejects_buffer_that_is_too_small(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = SENSOR_INTERNAL_TEMP;
    batch.sensors[0].timestamp = 0;
    batch.sensors[0].unit_id = UNIT_CELSIUS;
    batch.sensors[0].value = 0.0f;

    uint8_t buffer[13]{};
    memset(buffer, 0xCC, sizeof(buffer));
    size_t length = 123;

    TEST_ASSERT_FALSE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));
    TEST_ASSERT_EQUAL_UINT32(0, length);
    TEST_ASSERT_EQUAL_UINT8(0xCC, buffer[0]);
}

void test_protocol_rejects_count_larger_than_max_sensors(void) {
    SensorBatch_t batch{};
    batch.count = MAX_SENSORS + 1;

    uint8_t buffer[64]{};
    size_t length = 123;

    TEST_ASSERT_FALSE(Protocol_Encode(&batch, buffer, sizeof(buffer), &length));
    TEST_ASSERT_EQUAL_UINT32(0, length);
}

void test_protocol_rejects_null_arguments(void) {
    SensorBatch_t batch{};
    uint8_t buffer[4]{};
    size_t length = 123;

    TEST_ASSERT_FALSE(Protocol_Encode(nullptr, buffer, sizeof(buffer), &length));
    TEST_ASSERT_EQUAL_UINT32(0, length);

    length = 123;
    TEST_ASSERT_FALSE(Protocol_Encode(&batch, nullptr, sizeof(buffer), &length));
    TEST_ASSERT_EQUAL_UINT32(0, length);

    TEST_ASSERT_FALSE(Protocol_Encode(&batch, buffer, sizeof(buffer), nullptr));
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_protocol_frame_structure);
    RUN_TEST(test_protocol_checksum_calculation_simple);
    RUN_TEST(test_protocol_little_endian_float_encoding);
    RUN_TEST(test_protocol_zero_record_batch);
    RUN_TEST(test_protocol_multi_record_checksum_and_order);
    RUN_TEST(test_protocol_max_sensor_batch);
    RUN_TEST(test_protocol_timestamp_little_endian_encoding);
    RUN_TEST(test_protocol_checksum_wraparound);
    RUN_TEST(test_protocol_negative_and_fractional_float_encoding);
    RUN_TEST(test_protocol_rejects_buffer_that_is_too_small);
    RUN_TEST(test_protocol_rejects_count_larger_than_max_sensors);
    RUN_TEST(test_protocol_rejects_null_arguments);
    return UNITY_END();
}

