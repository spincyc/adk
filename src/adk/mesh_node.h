#pragma once

#include "link.h"
#include "object.h"
#include "serial_port.h"

namespace adk {

    // A Meshtastic radio, such as a Heltec WiFi LoRa 32 V3 running
    // Meshtastic, on one of the Mega's spare serial ports. Its Serial module,
    // set to TEXTMSG at 38400 baud, turns what the Mega sends into a text
    // message to everyone on the mesh's primary channel, and passes on every
    // text message it hears as a line, "NAME: text".
    //
    //   the board's RX (GPIO47) <- the Mega's TX pin, through 1 kΩ, with
    //                              2 kΩ from the board's pin to GND
    //   the board's TX (GPIO48) -> the Mega's RX pin
    //   GND -> GND; the board runs from its own USB cable
    //
    // Its pins take 3.3 V, so the Mega's TX is divided down. The node sends
    // what it has been given once the line has been quiet for a second, so
    // messages go at most one every 1.5 s.
    struct MeshNode : Object, Link
    {
        explicit MeshNode (HardwareSerial& port);

        // Send text to everyone on the channel. False, and nothing sent,
        // within 1.5 s of the last message, or if it is over 100 characters.
        bool send    (const char* text);
        bool canSend () const;

        // A message arrived in this update: who sent it, by their short name
        // of up to four letters, and what it said. Both stay until the next.
        bool        wasReceived () const;
        const char* sender      () const;
        const char* text        () const;

        // The longest message, in characters.
        static constexpr uint8_t MaxLength = 100;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool        sendLine  (const char* text) override;
        const char* heardLine () const override;

        HardwareSerial&       port_;
        LineReader<MaxLength> reader_;
        char                  sender_ [17];
        char                  text_   [MaxLength + 1];
        Millis                now_;
        Millis                sentAt_;
        bool                  sent_;
        bool                  received_;
    };
}
