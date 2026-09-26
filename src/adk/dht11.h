#pragma once

#include "timing.h"

namespace adk {

    // A DHT11 temperature and humidity sensor. On the kit's three-pin module,
    // wire S to the pin, + to 5 V and - to GND; the module has its own
    // pull-up. A bare four-pin DHT11 also needs 10 kohm from its data pin to
    // 5 V.
    //
    // The sensor needs a second after power-up and at least a second between
    // readings, so the first reading comes a second after the first update
    // and then one every two seconds. Receiving a reading blocks update () for
    // about 4 ms. Interrupts are held off while each bit is timed, a tenth of
    // a millisecond at a time, and let in at the start of the next, so
    // millis () keeps time and serial bytes and IR codes still arrive.
    struct Dht11 : Object
    {
        Dht11 (Pin pin);

        // The latest good reading: degrees Celsius and percent relative
        // humidity, both 0 until the first.
        float temperature () const;
        float humidity    () const;

        // A reading finished in this update, good or not: an event.
        bool measured () const;

        // Whether the latest reading arrived whole with a matching checksum.
        // A failed one leaves the values of the last good one.
        bool ok () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void receive ();

        StartTime signalled_;
        int16_t   temperature_;
        uint16_t  humidity_;
        Pin       pin_;
        bool      signalling_;
        bool      warm_;
        bool      ok_;
        bool      measured_;
    };
}
