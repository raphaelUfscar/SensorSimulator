#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h> // For unsigned long

unsigned long millis();
void set_mock_millis(unsigned long value);

#endif // MOCK_ARDUINO_H