#pragma once

#include "object.h"

namespace adk {

    // A DS18B20 digital thermometer: the kit's "18B20 Temp" module, or a bare
    // sensor or waterproof probe. It measures -55 to 125 degrees Celsius in
    // steps of 1/16 degree, and takes 750 ms over each reading.
    //
    //   Module  Mega
    //   S       the pin
    //   +       5V
    //   -       GND
    //
    // The module carries the 4.7 kohm pull-up resistor the data line needs. A
    // bare sensor or probe needs one fitted from its data wire to 5 V, and
    // its power wire on 5 V, not on GND ("parasite power" is not supported).
    // One sensor per pin.
    //
    // The first update () starts a reading and every reading starts the next,
    // so a new temperature arrives every 750 ms. The update () that collects
    // one blocks for about 10 ms, holding interrupts off for up to 70 us at a
    // time, and the rest return at once.
    struct Ds18b20 : Object
    {
        Ds18b20 (Pin pin);

        // The latest good reading, in degrees Celsius; 0 until the first.
        float celsius () const;

        // The latest reading arrived intact. False while the sensor is
        // missing or its data was garbled, and until the first reading.
        bool ok () const;

        // A reading arrived in this update.
        bool measured () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        enum struct Phase : uint8_t
        {
            Starting,
            Converting,
            Missing
        };

        void    collect ();
        bool    command (uint8_t function);
        bool    reset   ();
        uint8_t touch   (uint8_t byte);
        void    drive   ();
        void    release ();

        Millis  startedAt_;
        int16_t sixteenths_;
        Pin     pin_;
        Phase   phase_;
        bool    ok_;
        bool    measured_;
        bool    fresh_;
    };
}
