#include <Arduino.h>

#include "app_config.h"
#include "display.h"
#include "protocol.h"
#include "sensors.h"
#include "types.h"

// Main logic
SensorBatch_t batch;
uint32_t last_sample = 0;
uint32_t last_display = 0;

void setup() {
    Serial.begin(115200);
#if USE_DISPLAY
    Display_Init();
#endif
}

void loop() {
    uint32_t now = millis();
    if (now - last_sample >= SAMPLING_INTERVAL_MS) {
        last_sample = now;
        // Read all sensors
        batch.count = 0;
        Sensor_InternalTemp_Read(&batch.sensors[batch.count++]);
        Sensor_Voltage_Read(&batch.sensors[batch.count++]);
        Sensor_Current_Read(&batch.sensors[batch.count++]);
        Sensor_ExternalTemp_Read(&batch.sensors[batch.count++]);
        // Encode and transmit
        uint8_t buffer[256];  // Sufficient size
        size_t length;
        Protocol_Encode(&batch, buffer, &length);
        Serial.write(buffer, length);

#if USE_DISPLAY
        if (now - last_display >= DISPLAY_INTERVAL_MS) {
            last_display = now;
            Display_RenderBatch(&batch);
        }
#endif
        //Serial.printf("Temp: %.2f°C | Voltage: %.2fV | Current: %.2fA | ExtTemp: %.2f°C\n",
        //    batch.sensors[0].value, batch.sensors[1].value, 
        //    batch.sensors[2].value, batch.sensors[3].value);
    }
}