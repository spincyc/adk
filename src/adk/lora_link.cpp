#include "lora_link.h"

#include <string.h>

namespace adk {

    namespace {

        constexpr unsigned long Baud  = 9600;
        constexpr unsigned long Busy  = 1000;   // for AUX to rise
        constexpr unsigned long Reply = 200;    // for the settings to come back
        constexpr Millis        Calm  = 10;     // after AUX rises, before anything else

        // Save these settings: address 0, 9600 baud and 2.4 kbit/s on the
        // air, then the channel, then transparent sending, push-pull pins,
        // error correction on, and 10 mW.
        constexpr uint8_t Save    = 0xC0;
        constexpr uint8_t Speeds  = 0x1A;
        constexpr uint8_t Options = 0x47;
        constexpr uint8_t Read [] = {0xC1, 0xC1, 0xC1};
    }

    LoraLink::LoraLink (HardwareSerial& port, Pin mode, Pin aux, uint8_t channel)
        : port_     (port)
        , text_     {}
        , mode_     (mode)
        , aux_      (aux)
        , channel_  (channel)
        , ok_       (false)
        , received_ (false)
    {
    }

    void LoraLink::setup ()
    {
        ok_ = false;

        if (!claimSerial (port_) || !claimInput (mode_) || !claimInput (aux_))
        {
            return;
        }

        // Low whenever it is pulled; never high, which would put 5 V on it.
        digitalWrite (mode_, LOW);
        port_.begin (Baud);

        // M0 and M1 are let go, so high: settings mode.
        if (!ready ())
        {
            return;
        }

        const uint8_t wanted [] = {Save, 0x00, 0x00, Speeds, channel_, Options};

        while (port_.available () > 0)
        {
            port_.read ();
        }

        port_.write (wanted, sizeof wanted);
        ok_ = settings (wanted);

        // If the module didn't repeat them back, ask for them.
        if (!ok_)
        {
            port_.write (Read, sizeof Read);
            ok_ = settings (wanted);
        }

        pinMode (mode_, OUTPUT);
        ok_ = ready () && ok_;
    }

    bool LoraLink::ok () const
    {
        return ok_;
    }

    bool LoraLink::send (const char* text)
    {
        if (!ok_ || strlen (text) > MaxLength)
        {
            return false;
        }

        port_.print (text);
        port_.print ('\n');
        return true;
    }

    bool LoraLink::sendLine (const char* text)
    {
        return send (text);
    }

    const char* LoraLink::heardLine () const
    {
        return received_ ? text_ : nullptr;
    }

    bool LoraLink::wasReceived () const
    {
        return received_;
    }

    const char* LoraLink::text () const
    {
        return text_;
    }

    void LoraLink::update (Millis)
    {
        received_ = false;

        if (reader_.read (port_))
        {
            strncpy (text_, reader_.line (), MaxLength);
            text_[MaxLength] = '\0';
            received_        = true;
        }
    }

    // Wait for AUX to say the module is free, then a little longer, as a
    // change of mode only happens while it is.
    bool LoraLink::ready () const
    {
        unsigned long start = millis ();

        while (digitalRead (aux_) == LOW)
        {
            if (millis () - start >= Busy)
            {
                return false;
            }
        }

        delay (Calm);
        return true;
    }

    // The six bytes the module sends back: C0, then the five settings.
    bool LoraLink::settings (const uint8_t* expected)
    {
        uint8_t       reply [6] = {};
        uint8_t       count     = 0;
        unsigned long start     = millis ();

        while (count < sizeof reply && millis () - start < Reply)
        {
            if (port_.available () > 0)
            {
                reply[count++] = static_cast<uint8_t> (port_.read ());
            }
        }

        return count == sizeof reply && memcmp (reply, expected, sizeof reply) == 0;
    }
}
