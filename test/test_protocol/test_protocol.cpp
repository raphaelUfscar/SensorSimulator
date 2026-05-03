#include <unity.h>

#include <string.h>

#include "mocks.h"
#include "protocol.h"
#include "types.h"
#include "app_config.h"

void setUp(void) {}
void tearDown(void) {}

void test_protocol_frame_structure(void) {
    SensorBatch_t batch{};
    batch.count = 1;
    batch.sensors[0].sensor_id = 0x01;
    batch.sensors[0].timestamp = 1000; // 0x000003E8
    batch.sensors[0].unit_id = 0x01;
    batch.sensors[0].value = 25.5f;

    uint8_t buffer[64]{};
    size_t length = 0;
    Protocol_Encode(&batch, buffer, &length);

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
    Protocol_Encode(&batch, buffer, &length);

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
    Protocol_Encode(&batch, buffer, &length);

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

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_protocol_frame_structure);
    RUN_TEST(test_protocol_checksum_calculation_simple);
    RUN_TEST(test_protocol_little_endian_float_encoding);
    return UNITY_END();
}

