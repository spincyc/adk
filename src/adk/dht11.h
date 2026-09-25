#pragma once

#include "object.h"

namespace adk {

    // A DHT11 temperature and humidity sensor. On the kit's three-pin module,
    // wire S to the pin, + to 5 V and - to GND; the module has its own
    // pull-up. A bare four-pin DHT11 also needs 10 kohm from its data pin to
    // 5 V.
    //
    // The sensor needs a second after power-up and at least a second between
    // readings, so the first reading comes a second after the first update
    // and then one every two seconds. Receiving a reading blocks update () for
    // about 4 ms with interrupts off, so a serial byte or an IR code arriving
    // just then can be lost.
    struct Dht11 : Object
    {
        explicit Dht11 (Pin pin);

        // The latest good reading: degrees Celsius and percent relative
        // humidity, both 0 until the first.
        float temperature () const;
        float humidity    () const;

        // Whether the latest reading arrived whole with a matching checksum.
        // A failed one leaves the values of the last good one.
        bool ok () const;

        // A reading finished in this update, good or not.
        bool measured () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void receive ();

        Millis   signalledAt_;
        int16_t  temperature_;
        uint16_t humidity_;
        Pin      pin_;
        bool     signalling_;
        bool     ok_;
        bool     measured_;
        bool     starting_;
    };
}
