#pragma once

#include "object.h"
#include "timing.h"

namespace adk {

    // Where the radio is listened to: the band it tunes, how far apart
    // the stations are, and the treble cut broadcasters there expect.
    enum class FmBand : uint8_t
    {
        World,      // 87.5 to 108 MHz, stations 0.1 MHz apart
        Americas,   // 87.5 to 108 MHz, stations 0.2 MHz apart: 88.1, 88.3 ...
        Japan       // 76 to 90 MHz, stations 0.1 MHz apart
    };

    // An Si4703 FM radio board (the CJMCU-470, or SparkFun's), with its
    // crystal, a headphone amplifier and socket. The headphone cable is its
    // aerial, so it only finds stations with headphones plugged in.
    // Stations that send RDS also send their name and a line of text, such
    // as the song playing.
    //
    // The board runs on 3.3 V, and its pins must never see 5 V. The Mega
    // only ever pulls SDIO, SCLK and RST down to 0 V or lets go of them.
    // The board pulls SDIO and SCLK up to 3.3 V itself, but holds RST low,
    // so RST needs a 1 kΩ resistor to 3.3 V to lift it:
    //
    //   3.3V -> the Mega's 3.3V, GND -> GND
    //   SDIO, SCLK -> any two pins
    //   RST -> any pin, and 1 kΩ to 3.3 V
    //   SEN, GPIO1, GPIO2 -> not connected
    //
    // SDIO and SCLK are a two-wire bus like I2C, which ADK drives itself:
    // the Mega's own I2C pins, 20 and 21, have pull-ups to 5 V on the board.
    //
    // setup () takes about 0.6 s while the chip's crystal settles. Tuning
    // takes about 60 ms, and seeking a station up to a few seconds, while
    // the sketch carries on; the chip is left alone meanwhile, because
    // talking to it can knock it off tune. Reading its news holds update ()
    // up for about 1.5 ms every 40 ms.
    struct FmRadio : Object
    {
        FmRadio (Pin sdio, Pin sclk, Pin reset, FmBand band = FmBand::World);

        // The chip answered when setup () woke it.
        bool ok () const;

        // Tune to a frequency in tenths of a megahertz, tune (1011) for
        // 101.1 MHz, or move a number of stations' spacing up or down, as
        // step (encoder.turned ()). Both stay in the band, and wrap round
        // at its ends; a frequency between stations goes to the one below.
        // Asking for where it already is changes nothing, so both can be
        // called from every pass of loop ().
        void tune (uint16_t frequency);
        void step (int8_t stations);

        // Find the next station up or down the band.
        void seekUp   ();
        void seekDown ();

        // What it is tuned, or tuning, to, in tenths of a megahertz.
        uint16_t frequency () const;

        bool isTuning () const;

        // Tuning or seeking finished in this update.
        bool wasTuned () const;

        // How strong the station is, from 0 to 75 dBµV: above about 25 it
        // is clear, below 15 mostly hiss.
        uint8_t signal   () const;
        bool    isStereo () const;

        // From 0, silent, to 15. It starts at 8. Only a change is sent to
        // the radio, so it too can be called from every pass of loop ().
        void    setVolume (uint8_t volume);
        uint8_t volume    () const;

        // The station's name, eight characters, and its text, up to 64:
        // empty until heard, and emptied by tuning elsewhere.
        const char* stationName () const;
        const char* radioText   () const;

        // The name or text changed in this update.
        bool nameChanged () const;
        bool textChanged () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        enum class State : uint8_t { Off, Idle, Tuning, Settling };

        void     startTune  ();
        void     startSeek  (bool up);
        void     finishTune ();
        void     readRds    ();
        void     clearRds   ();
        uint16_t channels   () const;

        bool read  (uint8_t count);
        bool write (uint8_t last);

        // The two-wire bus, bit by bit.
        void pull    (Pin pin);
        void let     (Pin pin);
        bool begin   ();
        void end     ();
        bool send    (uint8_t byte);
        uint8_t take (bool more);

        uint16_t  registers_ [16];
        char      name_      [9];
        char      nextName_  [8];
        char      text_      [65];
        StartTime polled_;
        uint16_t  wanted_;
        uint16_t  channel_;
        Pin       sdio_;
        Pin       sclk_;
        Pin       reset_;
        FmBand    band_;
        State     state_;
        uint8_t   segments_;
        uint8_t   textFlag_;
        int8_t    seek_;
        bool      ok_;
        bool      tuned_;
        bool      named_;
        bool      texted_;
    };
}
