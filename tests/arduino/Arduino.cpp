#include "Arduino.h"

#include <stdio.h>
#include <stdlib.h>

volatile uint8_t  TCCR5A = 0;
volatile uint8_t  TCCR5B = 0;
volatile uint16_t ICR5   = 0;
volatile uint16_t OCR5A  = 0;
volatile uint16_t OCR5B  = 0;
volatile uint16_t OCR5C  = 0;
volatile uint16_t TCNT5  = 0;
arduino::WatchedRegister TCCR3A;
volatile uint8_t  TCCR3B = 0;
volatile uint16_t ICR3   = 0;
volatile uint16_t OCR3A  = 0;
volatile uint16_t OCR3B  = 0;
volatile uint16_t OCR3C  = 0;
volatile uint8_t  TCCR1A = 0;
volatile uint8_t  TCCR1B = 0;
volatile uint8_t  TIMSK1 = 0;
volatile uint16_t OCR1A  = 0;
volatile uint16_t TCNT1  = 0;

namespace arduino {

    // The TWI and SPI unit start afresh too (buses.cpp).
    void resetBuses ();

    std::function<int (uint8_t pin)>                                         onDigitalRead;
    std::function<void (uint8_t pin, uint8_t value)>                         onDigitalWrite;
    std::function<void (uint8_t pin, uint8_t mode)>                          onPinMode;
    std::function<unsigned long (uint8_t pin, uint8_t state, unsigned long)> onPulseIn;
    std::string                                                              shifted;
}

namespace {

    struct Handler
    {
        void (*function) ();
        int  mode;
    };

    arduino::PinState pins     [NUM_DIGITAL_PINS];
    Handler           handlers [6];
    unsigned long     nowUs    = 0;
    unsigned long     callCost = 0;
    unsigned long     step     = 0;
    unsigned long     seed     = 1;

    void charge ()
    {
        nowUs += callCost;
    }
}

namespace arduino {

    void reset ()
    {
        memset (pins,     0, sizeof pins);
        memset (handlers, 0, sizeof handlers);

        // Unconnected inputs read high, as if pulled up.
        for (auto& state : pins)
        {
            state.input = HIGH;
        }

        nowUs    = 0;
        callCost = 0;
        step     = 0;
        seed     = 1;

        onDigitalRead  = nullptr;
        onDigitalWrite = nullptr;
        onPinMode      = nullptr;
        onPulseIn      = nullptr;
        shifted.clear ();

        TCCR5A = 0;
        TCCR5B = 0;
        ICR5   = 0;
        OCR5A  = 0;
        OCR5B  = 0;
        OCR5C  = 0;
        TCNT5  = 0;
        TCCR1A = 0;
        TCCR1B = 0;
        TCCR3A.clear ();
        TCCR3B = 0;
        ICR3   = 0;
        OCR3A  = 0;
        OCR3B  = 0;
        OCR3C  = 0;
        TIMSK1 = 0;
        OCR1A  = 0;
        TCNT1  = 0;

        for (HardwareSerial* port : {&Serial, &Serial1, &Serial2, &Serial3})
        {
            port->clear ();
        }

        resetBuses ();
    }

    PinState& pin (uint8_t pin)
    {
        return pins[pin];
    }

    void drive (uint8_t pin, uint8_t level)
    {
        uint8_t previous = pins[pin].input;
        pins[pin].input  = level;

        int interrupt = digitalPinToInterrupt (pin);

        if (interrupt == NOT_AN_INTERRUPT || !handlers[interrupt].function || previous == level)
        {
            return;
        }

        int mode = handlers[interrupt].mode;

        bool rose = level == HIGH;

        if (mode == CHANGE || (mode == RISING && rose) || (mode == FALLING && !rose))
        {
            handlers[interrupt].function ();
        }
    }

    void advance (unsigned long ms)
    {
        nowUs += ms * 1000;
    }

    void advanceMicros (unsigned long us)
    {
        nowUs += us;
    }

    unsigned long now ()
    {
        return nowUs;
    }

    void setCallCost (unsigned long us)
    {
        callCost = us;
    }

    void setClockStep (unsigned long us)
    {
        step = us;
    }

    WatchedRegister& WatchedRegister::operator= (uint8_t next)
    {
        value = next;
        history.emplace_back (nowUs, value);
        return *this;
    }

    WatchedRegister& WatchedRegister::operator|= (uint8_t bits)
    {
        return *this = static_cast<uint8_t> (value | bits);
    }

    WatchedRegister& WatchedRegister::operator&= (uint8_t bits)
    {
        return *this = static_cast<uint8_t> (value & bits);
    }

    WatchedRegister::operator uint8_t () const
    {
        return value;
    }

    void WatchedRegister::clear ()
    {
        value = 0;
        history.clear ();
    }

    size_t Log::write (uint8_t byte)
    {
        text += static_cast<char> (byte);
        return 1;
    }
}

uint8_t digitalPinToTimer (uint8_t pin)
{
    static constexpr uint8_t timers [NUM_DIGITAL_PINS] = {
        NOT_ON_TIMER, NOT_ON_TIMER, TIMER3B, TIMER3C, TIMER0B, TIMER3A, TIMER4A,
        TIMER4B,      TIMER4C,      TIMER2B, TIMER2A, TIMER1A, TIMER1B, TIMER0A,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        TIMER5C, TIMER5B, TIMER5A};

    // The core reads its table in flash without checking the pin, and gets
    // whatever lies past the end. Stop the tests rather than pretend.
    if (pin >= NUM_DIGITAL_PINS)
    {
        fprintf (stderr, "digitalPinToTimer (%d): there is no such pin\n", pin);
        abort ();
    }

    return timers[pin];
}

void pinMode (uint8_t pin, uint8_t mode)
{
    pins[pin].mode = mode;

    if (arduino::onPinMode)
    {
        arduino::onPinMode (pin, mode);
    }
}

void digitalWrite (uint8_t pin, uint8_t value)
{
    charge ();
    pins[pin].output = value;

    if (arduino::onDigitalWrite)
    {
        arduino::onDigitalWrite (pin, value);
    }
}

int digitalRead (uint8_t pin)
{
    charge ();

    if (arduino::onDigitalRead)
    {
        return arduino::onDigitalRead (pin);
    }

    return pins[pin].input;
}

int analogRead (uint8_t pin)
{
    return pins[pin < NUM_ANALOG_INPUTS ? pin + A0 : pin].analog;
}

void analogWrite (uint8_t pin, int value)
{
    pins[pin].pwm = value;
}

void tone (uint8_t pin, unsigned int frequency, unsigned long)
{
    pins[pin].tone = frequency;
}

void noTone (uint8_t pin)
{
    pins[pin].tone = 0;
}

unsigned long pulseIn (uint8_t pin, uint8_t state, unsigned long timeout)
{
    return arduino::onPulseIn ? arduino::onPulseIn (pin, state, timeout) : 0;
}

// Record the byte as a 74HC595 would end up holding it, so sending it in the
// wrong bit order shows up in a test.
void shiftOut (uint8_t, uint8_t, uint8_t order, uint8_t value)
{
    if (order == LSBFIRST)
    {
        uint8_t reversed = 0;

        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            reversed = static_cast<uint8_t> (reversed | (((value >> bit) & 1) << (7 - bit)));
        }

        value = reversed;
    }

    arduino::shifted += static_cast<char> (value);
}

void attachInterrupt (uint8_t interrupt, void (*handler) (), int mode)
{
    handlers[interrupt] = {handler, mode};
}

void detachInterrupt (uint8_t interrupt)
{
    handlers[interrupt] = {nullptr, 0};
}

void interrupts ()
{
}

void noInterrupts ()
{
}

unsigned long millis ()
{
    nowUs += step;
    return nowUs / 1000;
}

unsigned long micros ()
{
    charge ();
    return nowUs;
}

void delay (unsigned long ms)
{
    nowUs += ms * 1000;
}

void delayMicroseconds (uint16_t us)
{
    nowUs += us;
}

long random (long high)
{
    return random (0, high);
}

long random (long low, long high)
{
    // A small linear congruential generator, so tests repeat exactly.
    seed = seed * 1103515245UL + 12345UL;
    if (high <= low)
    {
        return low;
    }

    unsigned long span = static_cast<unsigned long> (high - low);
    return low + static_cast<long> ((seed >> 16) % span);
}

void randomSeed (unsigned long value)
{
    seed = value;
}

size_t Print::write (const uint8_t* buffer, size_t size)
{
    size_t written = 0;

    while (size--)
    {
        written += write (*buffer++);
    }

    return written;
}

size_t Print::write (const char* text)
{
    return write (reinterpret_cast<const uint8_t*> (text), strlen (text));
}

size_t Print::print (const __FlashStringHelper* text)
{
    return write (reinterpret_cast<const char*> (text));
}

size_t Print::print (const char* text)
{
    return write (text);
}

size_t Print::print (char character)
{
    return write (static_cast<uint8_t> (character));
}

size_t Print::print (unsigned char value, int base)
{
    return print (static_cast<unsigned long> (value), base);
}

size_t Print::print (int value, int base)
{
    return print (static_cast<long> (value), base);
}

size_t Print::print (unsigned int value, int base)
{
    return print (static_cast<unsigned long> (value), base);
}

size_t Print::print (long value, int base)
{
    if (base == DEC && value < 0)
    {
        return print ('-') + print (static_cast<unsigned long> (-value), base);
    }

    return print (static_cast<unsigned long> (value), base);
}

size_t Print::print (unsigned long value, int base)
{
    char  digits [33];
    char* end = digits + sizeof digits - 1;
    *end      = '\0';

    do
    {
        unsigned long digit = value % static_cast<unsigned long> (base);
        *--end = static_cast<char> (digit < 10 ? '0' + digit : 'A' + digit - 10);
        value /= static_cast<unsigned long> (base);
    }
    while (value);

    return write (end);
}

size_t Print::print (double value, int digits)
{
    char text [48];
    snprintf (text, sizeof text, "%.*f", digits, value);
    return write (text);
}

size_t Print::println ()
{
    return write ("\r\n");
}

size_t Print::println (const __FlashStringHelper* text)
{
    return print (text) + println ();
}

size_t Print::println (const char* text)
{
    return print (text) + println ();
}

size_t Print::println (char character)
{
    return print (character) + println ();
}

size_t Print::println (unsigned char value, int base)
{
    return print (value, base) + println ();
}

size_t Print::println (int value, int base)
{
    return print (value, base) + println ();
}

size_t Print::println (unsigned int value, int base)
{
    return print (value, base) + println ();
}

size_t Print::println (long value, int base)
{
    return print (value, base) + println ();
}

size_t Print::println (unsigned long value, int base)
{
    return print (value, base) + println ();
}

size_t Print::println (double value, int digits)
{
    return print (value, digits) + println ();
}

HardwareSerial Serial;
HardwareSerial Serial1;
HardwareSerial Serial2;
HardwareSerial Serial3;

void HardwareSerial::begin (unsigned long rate)
{
    baud = rate;
}

void HardwareSerial::end ()
{
    baud = 0;
}

int HardwareSerial::available ()
{
    return static_cast<int> (input.size ());
}

int HardwareSerial::read ()
{
    if (input.empty ())
    {
        return -1;
    }

    int byte = static_cast<uint8_t> (input[0]);
    input.erase (0, 1);
    return byte;
}

void HardwareSerial::clear ()
{
    text.clear ();
    input.clear ();
    baud    = 0;
    onWrite = nullptr;
}

size_t HardwareSerial::write (uint8_t byte)
{
    text += static_cast<char> (byte);

    if (onWrite)
    {
        onWrite (byte);
    }

    return 1;
}

HardwareSerial::operator bool () const
{
    return true;
}

long map (long value, long fromLow, long fromHigh, long toLow, long toHigh)
{
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}
