#include <Arduino.h>
#include <TFT_eSPI.h>

#include "display.h"
#include "app_config.h"
#include "types.h"

#if USE_DISPLAY

// LilyGO T-Display S3 (LCD power + backlight enable)
static constexpr uint8_t PIN_POWER_ON = 15;
static constexpr uint8_t PIN_LCD_BL = 38;

static TFT_eSPI tft;

void Display_Init() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

    tft.init();
    tft.setRotation(1); // landscape
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println("Sensor Simulator");
}

void Display_RenderBatch(const SensorBatch_t* batch) {
    const int line_h = 24;

    auto printLine = [&](int y, const char* label, float value, const char* unit) {
        tft.fillRect(0, y, tft.width(), line_h, TFT_BLACK);
        tft.setCursor(0, y);
        tft.print(label);
        tft.print(": ");
        tft.print(value, 2);
        tft.print(" ");
        tft.print(unit);
    };

    for (uint8_t i = 0; i < batch->count; i++) {
        const SensorData_t* s = &batch->sensors[i];
        switch (s->sensor_id) {
            case SENSOR_INTERNAL_TEMP:  printLine(32,  "Int Temp", s->value, "C"); break;
            case SENSOR_VOLTAGE:        printLine(56,  "Voltage",  s->value, "V"); break;
            case SENSOR_CURRENT:        printLine(80,  "Current",  s->value, "A"); break;
            case SENSOR_EXTERNAL_TEMP:  printLine(104, "Ext Temp", s->value, "C"); break;
            default: break;
        }
    }
}

#else

void Display_Init() {}
void Display_RenderBatch(const SensorBatch_t*) {}

#endif

