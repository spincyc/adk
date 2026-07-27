#pragma once

#include <stdint.h>

constexpr uint8_t INPUT        = 0;
constexpr uint8_t OUTPUT       = 1;
constexpr uint8_t INPUT_PULLUP = 2;
constexpr uint8_t LOW          = 0;
constexpr uint8_t HIGH         = 1;
constexpr uint8_t LED_BUILTIN  = 13;

void pinMode      (uint8_t pin, uint8_t mode);
int  analogRead   (uint8_t pin);
void analogWrite  (uint8_t pin, int value);
int  digitalRead  (uint8_t pin);
void digitalWrite (uint8_t pin, uint8_t value);
void delay        (unsigned long intervalMs);
