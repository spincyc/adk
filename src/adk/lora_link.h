#pragma once

#include "link.h"
#include "object.h"
#include "serial_port.h"

namespace adk {

    // An Ebyte E32-433T20D LoRa module (sold as the E32-TTL-100 and under
    // other names): a 433 MHz radio that reaches a kilometer or more and
    // passes on whatever the Mega sends it to every module on its channel.
    // ADK sends and receives a line of text at a time.
    //
    //   VCC -> 5 V, GND -> GND
    //   RXD <- the Mega's TX pin, through 1 kΩ, with 2 kΩ from RXD to GND
    //   TXD -> the Mega's RX pin
    //   M0 and M1 -> joined together, to any pin
    //   AUX -> any pin
    //
    // Its pins work at 3.3 V, so the Mega's TX is divided down, and the Mega
    // only ever pulls M0 and M1 low or lets them go: the module pulls them
    // up itself. AUX is low while the module is busy.
    //
    // setup () lets M0 and M1 go high, which puts the module in its settings
    // mode, and gives it a channel, 410 MHz plus its number, and its lowest
    // power, 10 mW. Europe allows 10 mW without a license from 433.05 to
    // 434.79 MHz, so the channel is 24, 434 MHz, unless told otherwise; in
    // the USA, 433 MHz belongs to licensed radio amateurs. It then pulls
    // them low again for normal mode, taking about 0.2 s in all.
    struct LoraLink : Object, Link
    {
        LoraLink (HardwareSerial& port, Pin mode, Pin aux, uint8_t channel = 24);

        // The module took its settings at setup.
        bool ok () const;

        // Send a line of text to every module on the channel. False, and
        // nothing sent, if it is over 56 characters, one of the module's
        // packets with its newline.
        bool send (const char* text);

        // A line arrived in this update. Its text stays until the next.
        bool        wasReceived () const;
        const char* text        () const;

        // The longest line, in characters.
        static constexpr uint8_t MaxLength = 56;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool        sendLine  (const char* text) override;
        const char* heardLine () const override;

        bool ready    () const;
        bool settings (const uint8_t* expected);

        HardwareSerial& port_;
        LineReader<64>  reader_;
        char            text_ [MaxLength + 1];
        Pin             mode_;
        Pin             aux_;
        uint8_t         channel_;
        bool            ok_;
        bool            received_;
    };
}
