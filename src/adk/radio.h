#pragma once

#include "link.h"
#include "object.h"

namespace adk {

    struct RadioClock;

    // A 433 MHz radio link: a transmitter module and a receiver module,
    // sold in pairs. The transmitter's radio is on while its data pin is
    // high; the receiver's data pin follows what it hears, and between
    // messages that is noise.
    //
    //   Receiver (RX470C, or XY-MK-5V): VCC -> 5 V, GND -> GND,
    //     DATA -> any pin (either of its two)
    //   Transmitter (WL102-341): + -> 3.3 V, - -> GND, EN -> nothing,
    //     DAT -> any pin, through 1 kΩ, with 2 kΩ from DAT to GND
    //
    // The WL102 runs on 3.3 V, so its DAT pin gets the Mega's 5 V divided
    // down to 3.3 V. (An FS1000A runs on 5 V and needs no divider.) A wire
    // or coil on each module's antenna hole makes the range tens of meters;
    // without one it is a few.
    //
    // Messages of up to 60 characters go out as RadioHead's RH_ASK driver
    // sends them at its default 2000 bits a second, so a sketch can talk to
    // any Arduino running RadioHead. Each character becomes two 6-bit
    // symbols with as many ones as zeros, after a warm-up the receiver
    // locks onto, and a checksum rejects a message the noise got into.
    //
    // Both parts borrow Timer 1, whose interrupt samples the receiver 16,000
    // times a second, about a tenth of the processor's time, and steps the
    // transmitter along. A sketch has at most one of each, and while either
    // is declared pins 11 and 12 cannot do PWM.
    struct RadioTransmitter : Object, Link
    {
        explicit RadioTransmitter (Pin data);
        ~RadioTransmitter ();

        // Start sending a message, which goes out while the sketch carries
        // on: about 100 ms for a word. False, and nothing sent, while the
        // last message is still going or if this one is over 60 characters.
        bool send (const char* text);
        bool send (const uint8_t* bytes, uint8_t length);

        bool isSending () const;

        // The longest message, in characters.
        static constexpr uint8_t MaxLength = 60;

      protected:
        void setup () override;
        void stop  () override;

      private:
        friend struct RadioClock;

        bool        sendLine  (const char* text) override;
        const char* heardLine () const override;

        void step ();

        uint8_t          frame_ [MaxLength + 7];   // count, header, message, checksum
        uint8_t          length_;
        uint8_t          symbol_;
        uint8_t          bit_;
        uint8_t          tick_;
        Pin              pin_;
        volatile bool    sending_;
    };

    // The receiver listens all the time, and hands the sketch each message
    // that arrives whole.
    struct RadioReceiver : Object, Link
    {
        explicit RadioReceiver (Pin data);
        ~RadioReceiver ();

        // A whole, unbroken message arrived in this update.
        bool wasReceived () const;

        // The latest message, as text, and how many bytes it has. A message
        // of bytes rather than text may hold zeros, so read it with length ().
        const char* text   () const;
        uint8_t     length () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        friend struct RadioClock;

        bool        sendLine  (const char* text) override;
        const char* heardLine () const override;

        void sample ();

        uint8_t          frame_ [RadioTransmitter::MaxLength + 7];
        char             text_  [RadioTransmitter::MaxLength + 1];
        uint16_t         bits_;
        uint8_t          ramp_;
        uint8_t          integrator_;
        uint8_t          bitCount_;
        uint8_t          count_;
        uint8_t          heard_;
        uint8_t          length_;
        Pin              pin_;
        bool             last_;
        bool             active_;
        bool             received_;
        volatile bool    full_;
    };
}
