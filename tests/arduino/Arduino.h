#pragma once

// A host stand-in for the Arduino Mega core. Pins are plain memory, time only
// moves when a test says so, and hooks let a test play the part of a device.
// Types follow the AVR where it matters: unsigned int is 16 bits there, so
// delayMicroseconds () takes a uint16_t and an overflow fails to compile.

#include <functional>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

#define INPUT        0x0
#define OUTPUT       0x1
#define INPUT_PULLUP 0x2
#define LOW          0x0
#define HIGH         0x1
#define CHANGE       1
#define FALLING      2
#define RISING       3
#define LSBFIRST     0
#define MSBFIRST     1
#define DEC          10
#define HEX          16
#define BIN          2

#define NUM_DIGITAL_PINS  70
#define NUM_ANALOG_INPUTS 16
#define LED_BUILTIN       13
#define NOT_AN_INTERRUPT  -1

#define NOT_ON_TIMER 0
#define TIMER0A      1
#define TIMER0B      2
#define TIMER1A      3
#define TIMER1B      4
#define TIMER1C      5
#define TIMER2       6
#define TIMER2A      7
#define TIMER2B      8
#define TIMER3A      9
#define TIMER3B      10
#define TIMER3C      11
#define TIMER4A      12
#define TIMER4B      13
#define TIMER4C      14
#define TIMER4D      15
#define TIMER5A      16
#define TIMER5B      17
#define TIMER5C      18

static const uint8_t A0   = 54;
static const uint8_t A1   = 55;
static const uint8_t A2   = 56;
static const uint8_t A3   = 57;
static const uint8_t A4   = 58;
static const uint8_t A5   = 59;
static const uint8_t A6   = 60;
static const uint8_t A7   = 61;
static const uint8_t A8   = 62;
static const uint8_t A9   = 63;
static const uint8_t A10  = 64;
static const uint8_t A11  = 65;
static const uint8_t A12  = 66;
static const uint8_t A13  = 67;
static const uint8_t A14  = 68;
static const uint8_t A15  = 69;
static const uint8_t SDA  = 20;
static const uint8_t SCL  = 21;
static const uint8_t MISO = 50;
static const uint8_t MOSI = 51;
static const uint8_t SCK  = 52;
static const uint8_t SS   = 53;

#define analogInputToDigitalPin(p) (((p) < 16) ? (p) + 54 : -1)
#define digitalPinHasPWM(p)        (((p) >= 2 && (p) <= 13) || ((p) >= 44 && (p) <= 46))
#define digitalPinToInterrupt(p)                                                          \
    ((p) == 2 ? 0 : ((p) == 3 ? 1 : ((p) >= 18 && (p) <= 21 ? 23 - (p) : NOT_AN_INTERRUPT)))

#define PROGMEM
#define PSTR(text)            (text)
#define F(text)               (reinterpret_cast<const __FlashStringHelper*> (text))
#define pgm_read_byte(address) (*reinterpret_cast<const uint8_t*> (address))
#define pgm_read_word(address) (*reinterpret_cast<const uint16_t*> (address))
#define _BV(bit)              (1 << (bit))

class __FlashStringHelper;

uint8_t digitalPinToTimer (uint8_t pin);

void          pinMode           (uint8_t pin, uint8_t mode);
void          digitalWrite      (uint8_t pin, uint8_t value);
int           digitalRead       (uint8_t pin);
int           analogRead        (uint8_t pin);
void          analogWrite       (uint8_t pin, int value);
void          tone              (uint8_t pin, unsigned int frequency, unsigned long duration = 0);
void          noTone            (uint8_t pin);
unsigned long pulseIn           (uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
void          shiftOut          (uint8_t data, uint8_t clock, uint8_t order, uint8_t value);
void          attachInterrupt   (uint8_t interrupt, void (*handler) (), int mode);
void          detachInterrupt   (uint8_t interrupt);
void          interrupts        ();
void          noInterrupts      ();
unsigned long millis            ();
unsigned long micros            ();
void          delay             (unsigned long ms);
void          delayMicroseconds (uint16_t us);
long          map               (long value, long fromLow, long fromHigh, long toLow, long toHigh);
long          random            (long high);
long          random            (long low, long high);
void          randomSeed        (unsigned long seed);

// Arduino's min, max and constrain are macros; functions do the same here
// without clashing with the standard library.
constexpr auto min (auto a, auto b)
{
    return a < b ? a : b;
}

constexpr auto max (auto a, auto b)
{
    return a > b ? a : b;
}

constexpr auto constrain (auto value, auto low, auto high)
{
    return value < low ? low : (value > high ? high : value);
}

namespace arduino {

    // A register a test can watch: every value written to it, with the time
    // in microseconds, so a test can time what a part switched on and off.
    struct Register
    {
        Register& operator=  (uint8_t value);
        Register& operator|= (uint8_t bits);
        Register& operator&= (uint8_t bits);
        operator  uint8_t    () const;

        void clear ();

        uint8_t                                         value = 0;
        std::vector<std::pair<unsigned long, uint8_t>> history;
    };
}

// Timer 3, which an IrTransmitter runs as its 38 kHz carrier. Writes to
// TCCR3A, which connects the carrier to its pin, are watched.
extern arduino::Register  TCCR3A;
extern volatile uint8_t   TCCR3B;
extern volatile uint16_t  ICR3;
extern volatile uint16_t  OCR3A;
extern volatile uint16_t  OCR3B;
extern volatile uint16_t  OCR3C;

#define COM3C1 3
#define COM3B1 5
#define COM3A1 7
#define WGM31  1
#define WGM32  3
#define WGM33  4
#define CS30   0

// Timer 5, the 16-bit timer a Servo drives directly.
extern volatile uint8_t  TCCR5A;
extern volatile uint8_t  TCCR5B;
extern volatile uint16_t ICR5;
extern volatile uint16_t OCR5A;
extern volatile uint16_t OCR5B;
extern volatile uint16_t OCR5C;
extern volatile uint16_t TCNT5;

#define WGM50  0
#define WGM51  1
#define COM5C0 2
#define COM5C1 3
#define COM5B0 4
#define COM5B1 5
#define COM5A0 6
#define COM5A1 7
#define CS50   0
#define CS51   1
#define CS52   2
#define WGM52  3
#define WGM53  4

// Timer 1, which the 433 MHz radio runs as its sample clock, and the one
// interrupt it uses. A handler is a plain function here, so a test makes
// the interrupt happen by calling TIMER1_COMPA_vect ().
extern volatile uint8_t  TCCR1A;
extern volatile uint8_t  TCCR1B;
extern volatile uint8_t  TIMSK1;
extern volatile uint16_t OCR1A;
extern volatile uint16_t TCNT1;

#define CS10   0
#define CS11   1
#define CS12   2
#define WGM12  3
#define OCIE1A 1

#define ISR(vector) void vector ()

void TIMER1_COMPA_vect ();

class Print
{
  public:
    virtual ~Print () = default;

    virtual size_t write (uint8_t byte) = 0;
    virtual size_t write (const uint8_t* buffer, size_t size);

    size_t write (const char* text);

    size_t print (const __FlashStringHelper* text);
    size_t print (const char* text);
    size_t print (char character);
    size_t print (unsigned char value, int base = DEC);
    size_t print (int value, int base = DEC);
    size_t print (unsigned int value, int base = DEC);
    size_t print (long value, int base = DEC);
    size_t print (unsigned long value, int base = DEC);
    size_t print (double value, int digits = 2);

    size_t println ();
    size_t println (const __FlashStringHelper* text);
    size_t println (const char* text);
    size_t println (char character);
    size_t println (unsigned char value, int base = DEC);
    size_t println (int value, int base = DEC);
    size_t println (unsigned int value, int base = DEC);
    size_t println (long value, int base = DEC);
    size_t println (unsigned long value, int base = DEC);
    size_t println (double value, int digits = 2);
};

// The Mega's four serial ports: Serial on USB, and Serial1 to Serial3 on
// pins 18 and 19, 16 and 17, and 14 and 15. What a sketch sends is kept in
// text; a test puts what the device at the other end sends in input, and
// can answer each byte as it is sent with onWrite.
class HardwareSerial : public Print
{
  public:
    void   begin     (unsigned long baud);
    void   end       ();
    int    available ();
    int    read      ();
    size_t write     (uint8_t byte) override;

    using Print::write;

    explicit operator bool () const;

    // Forget everything sent and received.
    void clear ();

    std::string                         text;
    std::string                         input;
    unsigned long                       baud = 0;
    std::function<void (uint8_t byte)> onWrite;
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;

namespace arduino {

    struct PinState
    {
        uint8_t      mode;
        uint8_t      output;
        uint8_t      input;
        int          analog;
        int          pwm;
        unsigned int tone;
    };

    // Forget every pin, hook, interrupt and register, and set time to zero.
    void reset ();

    PinState& pin (uint8_t pin);

    // Drive an input pin as a device would, running any attached interrupt.
    void drive (uint8_t pin, uint8_t level);

    void          advance       (unsigned long ms);
    void          advanceMicros (unsigned long us);
    unsigned long now           ();

    // Charge this many microseconds for every pin read or write, so a
    // bit-banged protocol sees time pass while it polls.
    void setCallCost (unsigned long us);

    // Move time on by this many microseconds whenever millis () is read, so
    // a sketch waiting for time to pass gets there without a test's help.
    void setClockStep (unsigned long us);

    // Hooks a test sets to act as the device on the other end of a pin.
    extern std::function<int (uint8_t pin)>                                         onDigitalRead;
    extern std::function<void (uint8_t pin, uint8_t value)>                         onDigitalWrite;
    extern std::function<void (uint8_t pin, uint8_t mode)>                          onPinMode;
    extern std::function<unsigned long (uint8_t pin, uint8_t state, unsigned long)> onPulseIn;

    // Every byte shiftOut () sent, as a 74HC595 would hold it (most
    // significant bit in Q7), in order.
    extern std::string shifted;

    // Collects printed text, for testing anything that explains itself.
    struct Log : Print
    {
        size_t write (uint8_t byte) override;

        std::string text;
    };
}
