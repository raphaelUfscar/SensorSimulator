#include <unity.h>

#include "mock_arduino.h"
#include "mocks.h"
#include "sensors.h"
#include "hal.h"

void setUp(void) {
    set_mock_millis(0);
    set_mock_internal_temp(25.0f);
    set_mock_adc(1, 3.3f);
    set_mock_adc(2, 2.5f);
    set_mock_adc(3, 0.5f);
}

void tearDown(void) {}

void test_sensor_internal_temp_read(void) {
    SensorData_t data{};
    set_mock_millis(100);
    TEST_ASSERT_TRUE(Sensor_InternalTemp_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_INTERNAL_TEMP, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(100, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_CELSIUS, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(25.0f, data.value);
}

void test_sensor_internal_temp_read_uses_mocked_value(void) {
    SensorData_t data{};
    set_mock_millis(110);
    set_mock_internal_temp(-5.5f);

    TEST_ASSERT_TRUE(Sensor_InternalTemp_Read(&data));

    TEST_ASSERT_EQUAL_UINT8(SENSOR_INTERNAL_TEMP, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(110, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_CELSIUS, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(-5.5f, data.value);
}

void test_sensor_voltage_read(void) {
    SensorData_t data{};
    set_mock_millis(200);
    TEST_ASSERT_TRUE(Sensor_Voltage_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_VOLTAGE, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(200, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_VOLT, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(3.3f, data.value);
}

void test_sensor_voltage_read_zero_and_low_values(void) {
    SensorData_t data{};

    set_mock_millis(210);
    set_mock_adc(1, 0.0f);
    TEST_ASSERT_TRUE(Sensor_Voltage_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_VOLTAGE, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(210, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_VOLT, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.value);

    set_mock_millis(220);
    set_mock_adc(1, 0.125f);
    TEST_ASSERT_TRUE(Sensor_Voltage_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_VOLTAGE, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(220, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_VOLT, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(0.125f, data.value);
}

void test_sensor_current_read(void) {
    SensorData_t data{};
    set_mock_millis(300);
    TEST_ASSERT_TRUE(Sensor_Current_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_CURRENT, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(300, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_AMP, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.value);
}

void test_sensor_current_read_positive_negative_and_larger_values(void) {
    SensorData_t data{};

    set_mock_millis(310);
    set_mock_adc(2, 2.6f);
    TEST_ASSERT_TRUE(Sensor_Current_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_CURRENT, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(310, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_AMP, data.unit_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, data.value);

    set_mock_millis(320);
    set_mock_adc(2, 2.4f);
    TEST_ASSERT_TRUE(Sensor_Current_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_CURRENT, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(320, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_AMP, data.unit_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, data.value);

    set_mock_millis(330);
    set_mock_adc(2, 3.0f);
    TEST_ASSERT_TRUE(Sensor_Current_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_CURRENT, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(330, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_AMP, data.unit_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, data.value);
}

void test_sensor_external_temp_read(void) {
    SensorData_t data{};
    set_mock_millis(400);
    TEST_ASSERT_TRUE(Sensor_ExternalTemp_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_EXTERNAL_TEMP, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(400, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_CELSIUS, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, data.value);
}

void test_sensor_external_temp_read_zero_and_fractional_values(void) {
    SensorData_t data{};

    set_mock_millis(410);
    set_mock_adc(3, 0.0f);
    TEST_ASSERT_TRUE(Sensor_ExternalTemp_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_EXTERNAL_TEMP, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(410, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_CELSIUS, data.unit_id);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.value);

    set_mock_millis(420);
    set_mock_adc(3, 0.255f);
    TEST_ASSERT_TRUE(Sensor_ExternalTemp_Read(&data));
    TEST_ASSERT_EQUAL_UINT8(SENSOR_EXTERNAL_TEMP, data.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(420, data.timestamp);
    TEST_ASSERT_EQUAL_UINT8(UNIT_CELSIUS, data.unit_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.5f, data.value);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_sensor_internal_temp_read);
    RUN_TEST(test_sensor_internal_temp_read_uses_mocked_value);
    RUN_TEST(test_sensor_voltage_read);
    RUN_TEST(test_sensor_voltage_read_zero_and_low_values);
    RUN_TEST(test_sensor_current_read);
    RUN_TEST(test_sensor_current_read_positive_negative_and_larger_values);
    RUN_TEST(test_sensor_external_temp_read);
    RUN_TEST(test_sensor_external_temp_read_zero_and_fractional_values);
    return UNITY_END();
}

