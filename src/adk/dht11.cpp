#include "dht11.h"

#include <Arduino.h>

namespace adk {

    namespace {

        const Millis Period = 2000;
        const Millis WarmUp = 1000;

        // The datasheet asks for at least 18 ms. millis () sometimes counts
        // in steps of two, so 20 on its clock is sure to be 18 in fact.
        const Millis StartSignal = 20;

        // Every level of the reply lasts under 100 us, so one that lasts
        // 150 us means the sensor is missing or has stopped. A high longer
        // than 50 us is a 1.
        const uint16_t Timeout  = 150;
        const uint16_t OneAbove = 50;

        // Microseconds since began. With interrupts off, micros () no longer
        // counts overflows of Timer 0, but its low ten bits still tick every
        // 4 us, and every wait here is far shorter than their 1024 us span.
        uint16_t since (uint16_t began)
        {
            return static_cast<uint16_t> ((static_cast<uint16_t> (micros ()) - began) & 1023);
        }

        // How long the line stays at a level, or more than Timeout.
        uint16_t hold (Pin pin, int level)
        {
            uint16_t began = static_cast<uint16_t> (micros ());

            for (;;)
            {
                bool     changed = digitalRead (pin) != level;
                uint16_t elapsed = since (began);

                if (changed || elapsed > Timeout)
                {
                    return elapsed;
                }
            }
        }

        // The sensor answers the start signal 20-40 us after it ends: low for
        // 80 us, high for 80 us, then 40 bits, most significant first. Each
        // bit is low for 50 us, then high for 26-28 us for a 0 or 70 us for a 1.
        bool listen (Pin pin, uint8_t (&bytes) [5])
        {
            if (hold (pin, HIGH) > Timeout || hold (pin, LOW) > Timeout
                || hold (pin, HIGH) > Timeout)
            {
                return false;
            }

            for (uint8_t bit = 0; bit < 40; ++bit)
            {
                if (hold (pin, LOW) > Timeout)
                {
                    return false;
                }

                uint16_t high = hold (pin, HIGH);

                if (high > Timeout)
                {
                    return false;
                }

                bytes[bit / 8] = static_cast<uint8_t> ((bytes[bit / 8] << 1) | (high > OneAbove));
            }

            return true;
        }
    }

    Dht11::Dht11 (Pin pin)
        : signalledAt_ (0)
        , temperature_ (0)
        , humidity_    (0)
        , pin_         (pin)
        , signalling_  (false)
        , ok_          (false)
        , measured_    (false)
        , starting_    (true)
    {
    }

    void Dht11::setup ()
    {
        claimInput (pin_, true);
    }

    float Dht11::temperature () const
    {
        return temperature_ / 10.0f;
    }

    float Dht11::humidity () const
    {
        return humidity_ / 10.0f;
    }

    bool Dht11::ok () const
    {
        return ok_;
    }

    bool Dht11::measured () const
    {
        return measured_;
    }

    void Dht11::update (Millis now)
    {
        measured_ = false;

        // Count as if a reading began a second before the first update, so
        // the first real one waits the second the sensor needs after power-up.
        if (starting_)
        {
            signalledAt_ = now - (Period - WarmUp);
            starting_    = false;
        }

        if (!signalling_ && now - signalledAt_ >= Period)
        {
            // The start signal holds the line low. Setting the level first
            // means the pin never drives it high.
            digitalWrite (pin_, LOW);
            pinMode      (pin_, OUTPUT);
            signalledAt_ = now;
            signalling_  = true;
        }
        else if (signalling_ && now - signalledAt_ >= StartSignal)
        {
            receive ();
            signalling_ = false;
            measured_   = true;
        }
    }

    void Dht11::stop ()
    {
        // Let go of the line; the next reading comes a period after this one
        // began.
        if (signalling_)
        {
            pinMode (pin_, INPUT_PULLUP);
            signalling_ = false;
        }
    }

    void Dht11::receive ()
    {
        uint8_t bytes [5] = {};

        noInterrupts ();
        pinMode (pin_, INPUT_PULLUP);
        bool whole = listen (pin_, bytes);
        interrupts ();

        uint8_t sum = static_cast<uint8_t> (bytes[0] + bytes[1] + bytes[2] + bytes[3]);
        ok_         = whole && sum == bytes[4];

        if (!ok_)
        {
            return;
        }

        // Whole units, then tenths. Bit 7 of the tenths of a degree marks a
        // temperature below zero.
        int16_t degrees = static_cast<int16_t> (bytes[2] * 10 + (bytes[3] & 0x7F));

        humidity_    = static_cast<uint16_t> (bytes[0] * 10 + bytes[1]);
        temperature_ = (bytes[3] & 0x80) ? static_cast<int16_t> (-degrees) : degrees;
    }
}
