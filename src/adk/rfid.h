#pragma once

#include "timing.h"

namespace adk {

    // The RC522 RFID reader module (an NXP MFRC522 on the SPI bus) and the
    // kit's card and key fob. Each tag holds a unique ID (UID) that the
    // reader picks up when the tag is within a few centimeters of it.
    //
    // THE MODULE RUNS ON 3.3 V. Connect its 3.3V pin to the Mega's 3.3V pin,
    // never to 5V:
    //
    //   Module  Mega
    //   SDA     the select pin (it is the SPI chip select), 53 or any free pin
    //   SCK     52
    //   MOSI    51
    //   MISO    50
    //   IRQ     not connected
    //   GND     GND
    //   RST     the reset pin
    //   3.3V    3.3V
    //
    // Protect SDA, SCK, MOSI and RST from the Mega's 5 V signals. Each of
    // those four lines needs a divider: 1 kohm from the Mega's pin to the
    // reader's pin, and 2 kohm from the reader's pin to GND. A suitable level
    // shifter also works. Never connect these inputs directly to the Mega.
    // MISO needs no divider: 3.3 V is already a high for the Mega.
    //
    // The reader looks for a tag every 100 ms. It reads 4-byte UIDs, which is
    // what MIFARE Classic tags like the kit's have. A tag with a 7-byte UID
    // (MIFARE Ultralight, NTAG stickers) is not read at all, and neither are
    // two tags held to the reader at once. update () never waits for the
    // radio, and takes at most about 0.3 ms. setup () takes 50 ms.
    struct Rfid : Object
    {
        Rfid (Pin select, Pin reset);

        // A tag arrived in this update: once per presentation, not while it
        // stays. A different tag replacing it counts as a new arrival.
        bool wasRead () const;

        // A tag is in the field now. It stays true through one missed look,
        // and turns false on the second.
        bool isPresent () const;

        // The UID of the latest tag, its first byte most significant, so a
        // sketch can write `if (rfid.uid () == 0x1A2B3C4D)`. It is kept after
        // the tag leaves, and is 0 until one has been read.
        uint32_t uid () const;

        // The reader answered at setup ().
        bool ok () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

        // Switch the radio field off. The reader stays off until setup ().
        void stop () override;

      private:
        enum class Step : uint8_t
        {
            Off,
            Resting,
            Waking,
            Listing
        };

        void    look          ();
        void    listen        (Millis now);
        void    found         (uint32_t uid);
        void    missed        ();
        void    field         (bool on);
        void    send          (const uint8_t* frame, uint8_t length, uint8_t lastBits,
                               uint8_t command);
        uint8_t reply         (uint8_t* bytes, uint8_t capacity);
        uint8_t readRegister  (uint8_t address);
        void    writeRegister (uint8_t address, uint8_t value);

        StartTime looked_;
        uint32_t  uid_;
        Pin       select_;
        Pin       reset_;
        Step      step_;
        uint8_t   misses_;
        bool      ok_;
        bool      present_;
        bool      wasRead_;
    };
}
