#pragma once

#include "link.h"
#include "object.h"
#include "serial_port.h"

namespace adk {

    enum struct LoraSpeed : uint8_t
    {
        Far,        // spreading factor 10: about 0.3 s a message, a few km
        Quick       // spreading factor 7: about 0.05 s a message, about a km
    };

    // How a modem is set up. Name only what differs, as in
    // {.partner = 2, .speed = adk::LoraSpeed::Quick}.
    struct LoraSettings
    {
        uint16_t  partner = 0;              // where send (text) goes; 0 for every modem
        LoraSpeed speed   = LoraSpeed::Far;
        uint8_t   power   = 15;             // dBm, from 0 to 15
        uint8_t   network = 6;              // 1 to 15: modems hear only their own
        uint32_t  band    = 915000000;      // Hz: 915 MHz in the Americas
    };

    // A REYAX RYLR896 LoRa modem: a radio on 915 MHz (868 in Europe) that
    // reaches a kilometre or more, commanded in words over one of the
    // Mega's spare serial ports. Each modem has an address, and modems with
    // the same network number hear each other.
    //
    //   VDD  -> 3.3 V from the breadboard power module
    //   GND  -> GND
    //   RXD  <- the Mega's TX pin, through 1 kΩ, with 2 kΩ from RXD to GND
    //   TXD  -> the Mega's RX pin
    //   NRST -> not connected
    //
    // It draws 50 mA while it sends at full power, all the Mega's 3.3V pin
    // can give, so it gets its own supply, and its pins take 3.3 V, so the
    // Mega's TX is divided down. setup () gives it its address, network,
    // band, speed and power, taking about 60 ms: the modem forgets all but
    // its address and network when it restarts. At the Far speed, REYAX's
    // choice for up to 3 km, a short message takes about 0.3 s on the air;
    // Quick trades range for about 0.05 s, for a Bridge that steers things.
    struct LoraModem : Object, Link
    {
        LoraModem (HardwareSerial& port, uint16_t address, LoraSettings settings = {});

        // The modem answered at setup.
        bool ok () const;

        // Send text to the modem at an address, or to every modem on the
        // network with address 0; or to the partner in its settings. False,
        // and nothing sent, while the last message is still going or if
        // this one is over 60 characters.
        bool send (uint16_t to, const char* text);
        bool send (const char* text);
        bool isSending () const;

        // A message arrived in this update. Its text, and the address it
        // came from, stay until the next.
        bool        wasReceived () const;
        uint16_t    sender      () const;
        const char* text        () const;

        // How strong it arrived, in dBm: about -40 across a room, and -120 at
        // the edge of range. The margin above the noise, in dB, goes below 0
        // near the edge; LoRa still hears down to about -15.
        int16_t signal () const;
        int8_t  margin () const;

        // The longest message, in characters.
        static constexpr uint8_t MaxLength = 60;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool        sendLine  (const char* text) override;
        const char* heardLine () const override;

        bool command (const char* line);
        bool heard   (const char* fields);

        HardwareSerial& port_;
        LineReader<100> reader_;
        char            text_ [MaxLength + 1];
        LoraSettings    settings_;
        Millis          sentAt_;
        uint16_t        address_;
        uint16_t        sender_;
        int16_t         signal_;
        int8_t          margin_;
        bool            ok_;
        bool            sending_;
        bool            starting_;
        bool            received_;
    };
}
