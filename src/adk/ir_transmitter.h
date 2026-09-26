#pragma once

#include "object.h"

namespace adk {

    // An infrared LED module (the 37-in-1 kit's KY-005) that sends NEC codes,
    // as the kit's remote does: so the Mega can work a TV, or talk to
    // another board's IrReceiver across a room.
    //
    //   S -> pin 2, 3 or 5, through 220 Ω (not every module has its own)
    //   middle pin -> not connected, - -> GND
    //
    // A receiver only listens for light flashing 38,000 times a second.
    // Timer 3 makes that flashing on its own pins, 2, 3 and 5, so none of
    // them can do PWM while a transmitter is declared.
    //
    // send () holds the sketch for about 70 ms while the code goes out, as
    // the ultrasonic sensor holds it for an echo; interrupts stay on.
    struct IrTransmitter : Object
    {
        explicit IrTransmitter (Pin pin);

        // Send a button's code, such as remote::power, to the remote's
        // address; an address above 255 is sent whole, as some remotes do.
        void send (uint8_t command, uint16_t address = 0);

        // Send the short code a remote repeats while a button is held.
        void repeat ();

      protected:
        void setup () override;
        void stop  () override;

      private:
        void carrier (bool on);
        void play    (const uint16_t* widths, uint8_t count);

        uint8_t output_;        // the timer's bit that connects the carrier to the pin
        Pin     pin_;
        bool    ready_;
    };
}
