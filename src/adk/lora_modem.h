#pragma once

#include "object.h"
#include "serial_port.h"

namespace adk {

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
    // It draws 50 mA while it sends, all the Mega's 3.3V pin can give, so it
    // gets its own supply, and its pins take 3.3 V, so the Mega's TX is
    // divided down. setup () gives it its address, network and band, and
    // REYAX's settings for up to 3 km, taking about 50 ms: the modem
    // forgets its band and settings when it restarts. A short message then
    // takes about 0.3 s on the air.
    struct LoraModem : Object
    {
        LoraModem (HardwareSerial& port, uint16_t address, uint8_t network = 6,
                   uint32_t band = 915000000);

        // The modem answered at setup.
        bool ok () const;

        // Send text to the modem at an address, or to every modem on the
        // network with address 0. False, and nothing sent, while the last
        // message is still going or if this one is over 60 characters.
        bool send (uint16_t to, const char* text);
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

        static constexpr uint8_t MaxLength = 60;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool command (const char* line);
        bool heard   (const char* fields);

        HardwareSerial& port_;
        LineReader<100> reader_;
        char            text_ [MaxLength + 1];
        uint32_t        band_;
        Millis          sentAt_;
        uint16_t        address_;
        uint16_t        sender_;
        int16_t         signal_;
        int8_t          margin_;
        uint8_t         network_;
        bool            ok_;
        bool            sending_;
        bool            starting_;
        bool            received_;
    };
}
