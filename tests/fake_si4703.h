#pragma once

// Plays an Si4703 FM radio chip on three pins, as FmRadio talks to it: a
// two-wire bus on SDIO and SCLK that the Mega only ever pulls low or lets
// go, and RST. It watches every pin change, answers on SDIO bit by bit,
// tunes and seeks among the stations a test lists, taking as long as the
// real chip, and hands out the RDS groups a test queues. It notes whether
// the Mega ever put 5 V on one of its pins.

#include <adk/board.h>

#include <stdint.h>
#include <vector>

namespace fake {

    struct Si4703
    {
        struct Station
        {
            uint16_t channel;
            uint8_t  strength;      // dBµV
            bool     stereo;
        };

        // One RDS group: its four blocks, and how many errors were
        // corrected in blocks B, C and D (0 to 3, 3 meaning too many).
        struct Group
        {
            uint16_t a, b, c, d;
            uint8_t  errorsB = 0;
            uint8_t  errorsC = 0;
            uint8_t  errorsD = 0;
        };

        Si4703 (adk::Pin sdio, adk::Pin sclk, adk::Pin reset);
        ~Si4703 ();

        Si4703            (const Si4703&) = delete;
        Si4703& operator= (const Si4703&) = delete;

        uint16_t             registers [16];
        std::vector<Station> stations;
        std::vector<Group>   groups;           // handed out one per read of the news
        unsigned long        tuneMicros;       // how long a tune or seek takes
        bool                 twoWire;          // chosen as RST rose
        bool                 drivenHigh;       // the Mega put 5 V on one of its pins

      private:
        enum struct Phase : uint8_t { Idle, Address, Write, Read, Ignore };

        bool mine        (uint8_t pin) const;
        bool pulled      (uint8_t pin) const;
        void change      ();
        void rise        ();
        void fall        ();
        void heard       (uint8_t byte);
        void written     (uint8_t reg, uint16_t before);
        void startRead   ();
        void loadByte    ();
        void settle      ();
        uint8_t strength (uint16_t channel) const;

        adk::Pin      sdio_, sclk_, reset_;
        Phase         phase_;
        Phase         next_;
        uint8_t       clocks_;
        uint8_t       shift_;
        uint8_t       index_;
        uint8_t       high_;
        uint8_t       byte_;
        bool          holding_;      // the chip pulls SDIO low
        bool          ack_;
        bool          scl_, sda_, rst_;
        bool          busy_;
        unsigned long doneAt_;
        uint16_t      result_;
        bool          failed_;
    };
}
