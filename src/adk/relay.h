#pragma once

#include "digital.h"

namespace adk {

    // A 5 V relay module, such as the one in the 37-in-1 sensor kit. A small
    // coil pulls a switch closed, so the pin can turn a separate circuit on
    // and off. It clicks, and its LED lights, when the relay is on.
    //
    //   S -> the pin
    //   + -> 5 V
    //   - -> GND
    //
    // The switched circuit goes through the COM and NO (normally open)
    // terminals, so it is off whenever the relay is off.
    //
    // Never connect mains. Switch only low-voltage DC loads, such as an LED
    // and its resistor or a small lamp on a breadboard power module.
    //
    // Most kit modules are active high. If yours switches on when the sketch
    // starts and off when told on, it is active low.
    struct Relay : Object
    {
        explicit Relay (Pin pin, Polarity polarity = ActiveHigh);

        void on     ();
        void off    ();
        void toggle ();
        bool isOn   () const;

      protected:
        void setup () override;
        void stop  () override;

      private:
        void set (bool on);

        Pin      pin_;
        Polarity polarity_;
        bool     on_;
    };
}
