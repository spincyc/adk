#include <Arduino.h>

#include <array>

namespace {
    constexpr std::size_t pinCount = 256;

    struct State
    {
        std::array<uint8_t, pinCount>              modes;
        std::array<int, pinCount>                  analogInputs;
        std::array<int, pinCount>                  analogOutputs;
        std::array<uint8_t, pinCount>              digitalInputs;
        std::array<uint8_t, pinCount>              digitalOutputs;
        uint64_t                                   timeUs;
        std::vector<adk::test::arduino::Operation> operations;
    };

    State state {};

    void record (
        adk::test::arduino::OperationKind kind,
        uint8_t                           pin,
        int                               value)
    {
        state.operations.push_back ({kind, pin, value, state.timeUs});
    }
}

void pinMode (uint8_t pin, uint8_t mode)
{
    state.modes[pin] = mode;
    record (adk::test::arduino::OperationKind::PinMode, pin, mode);
}

int analogRead (uint8_t pin)
{
    auto value = state.analogInputs[pin];
    record (adk::test::arduino::OperationKind::AnalogRead, pin, value);
    return value;
}

void analogWrite (uint8_t pin, int value)
{
    state.analogOutputs[pin] = value;
    record (adk::test::arduino::OperationKind::AnalogWrite, pin, value);
}

int digitalRead (uint8_t pin)
{
    auto value = state.digitalInputs[pin];
    record (adk::test::arduino::OperationKind::DigitalRead, pin, value);
    return value;
}

void digitalWrite (uint8_t pin, uint8_t value)
{
    state.digitalOutputs[pin] = value;
    record (adk::test::arduino::OperationKind::DigitalWrite, pin, value);
}

void delay (unsigned long intervalMs)
{
    record (
        adk::test::arduino::OperationKind::Delay,
        0,
        static_cast<int> (intervalMs));
    state.timeUs += static_cast<uint64_t> (intervalMs) * 1000U;
}

void delayMicroseconds (unsigned int intervalUs)
{
    record (adk::test::arduino::OperationKind::DelayMicroseconds, 0,
            static_cast<int> (intervalUs));
    state.timeUs += intervalUs;
}

unsigned long millis ()
{
    return static_cast<unsigned long> (static_cast<uint32_t> (state.timeUs / 1000U));
}

unsigned long micros ()
{
    return static_cast<unsigned long> (static_cast<uint32_t> (state.timeUs));
}

namespace adk { namespace test { namespace arduino {

    void reset ()
    {
        state = {};
    }

    void clearTrace ()
    {
        state.operations.clear ();
    }

    void setAnalogInput (uint8_t pin, int value)
    {
        state.analogInputs[pin] = value;
    }

    void setDigitalInput (uint8_t pin, uint8_t value)
    {
        state.digitalInputs[pin] = value;
    }

    void setTimeUs (uint64_t value)
    {
        state.timeUs = value;
    }

    void advanceTimeUs (uint64_t interval)
    {
        state.timeUs += interval;
    }

    uint8_t mode (uint8_t pin)
    {
        return state.modes[pin];
    }

    int analogOutput (uint8_t pin)
    {
        return state.analogOutputs[pin];
    }

    uint8_t digitalOutput (uint8_t pin)
    {
        return state.digitalOutputs[pin];
    }

    uint64_t timeUs ()
    {
        return state.timeUs;
    }

    const std::vector<Operation>& trace ()
    {
        return state.operations;
    }

}}}
