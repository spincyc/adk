#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

constexpr uint8_t INPUT          = 0;
constexpr uint8_t OUTPUT         = 1;
constexpr uint8_t INPUT_PULLUP   = 2;
constexpr uint8_t INPUT_PULLDOWN = 3;
constexpr uint8_t LOW            = 0;
constexpr uint8_t HIGH           = 1;
constexpr uint8_t LED_BUILTIN    = 13;
constexpr uint8_t NUM_DIGITAL_PINS = 70;

void          pinMode            (uint8_t pin, uint8_t mode);
int           analogRead         (uint8_t pin);
void          analogWrite        (uint8_t pin, int value);
int           digitalRead        (uint8_t pin);
void          digitalWrite       (uint8_t pin, uint8_t value);
void          delay              (unsigned long intervalMs);
void          delayMicroseconds  (unsigned int intervalUs);
unsigned long millis             ();
unsigned long micros             ();

namespace adk { namespace test { namespace arduino {

    enum struct OperationKind : uint8_t
    {
        PinMode,
        AnalogRead,
        AnalogWrite,
        DigitalRead,
        DigitalWrite,
        Delay,
        DelayMicroseconds
    };

    struct Operation
    {
        OperationKind kind;
        uint8_t       pin;
        int           value;
        uint64_t      timeUs;
    };

    void reset           ();
    void clearTrace      ();
    void setAnalogInput  (uint8_t pin, int value);
    void setDigitalInput (uint8_t pin, uint8_t value);
    void setTimeUs       (uint64_t value);
    void advanceTimeUs   (uint64_t interval);

    uint8_t  mode          (uint8_t pin);
    int      analogOutput  (uint8_t pin);
    uint8_t  digitalOutput (uint8_t pin);
    uint64_t timeUs        ();

    const std::vector<Operation>& trace ();

}}}
