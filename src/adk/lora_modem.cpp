#include "lora_modem.h"

#include "print.h"

#include <stdlib.h>
#include <string.h>

namespace adk {

    namespace {

        constexpr unsigned long Baud   = 115200;
        constexpr unsigned long Waking = 1000;  // for a modem still starting up
        constexpr unsigned long Answer = 200;   // for +OK to a command
        constexpr Millis        GiveUp = 3000;  // for +OK to a message

        // Spreading factor, bandwidth (7: 125 kHz), coding rate 4/5 and
        // preamble. Far is REYAX's choice for up to 3 km.
        constexpr const char* Speeds [] = {"AT+PARAMETER=10,7,1,7", "AT+PARAMETER=7,7,1,4"};
    }

    LoraModem::LoraModem (HardwareSerial& port, uint16_t address, LoraSettings settings)
        : port_     (port)
        , text_     {}
        , settings_ (settings)
        , address_  (address)
        , sender_   (0)
        , signal_   (0)
        , margin_   (0)
        , ok_       (false)
        , sending_  (false)
        , received_ (false)
    {
    }

    void LoraModem::setup ()
    {
        ok_      = false;
        sending_ = false;

        if (!claimSerial (port_))
        {
            return;
        }

        port_.begin (Baud);

        unsigned long start = millis ();

        while (!ok_ && millis () - start < Waking)
        {
            ok_ = command ("AT");
        }

        Text<32> address;
        Text<32> network;
        Text<32> band;
        Text<32> power;
        print (address, "AT+ADDRESS=", address_);
        print (network, "AT+NETWORKID=", settings_.network);
        print (band, "AT+BAND=", settings_.band);
        print (power, "AT+CRFOP=", min (settings_.power, uint8_t {15}));

        ok_ = ok_ && command (address.c_str ()) && command (network.c_str ())
                  && command (band.c_str ()) && command (power.c_str ())
                  && command (Speeds[static_cast<uint8_t> (settings_.speed)]);
    }

    bool LoraModem::ok () const
    {
        return ok_;
    }

    bool LoraModem::send (uint16_t to, const char* text)
    {
        size_t length = strlen (text);

        if (!ok_ || sending_ || length > MaxLength)
        {
            return false;
        }

        print (port_, "AT+SEND=", to, ',', length, ',', text, "\r\n");
        sending_ = true;
        sent_.restart ();
        return true;
    }

    bool LoraModem::send (const char* text)
    {
        return send (settings_.partner, text);
    }

    bool LoraModem::sendLine (const char* text)
    {
        return send (text);
    }

    const char* LoraModem::heardLine () const
    {
        return received_ ? text_ : nullptr;
    }

    bool LoraModem::isSending () const
    {
        return sending_;
    }

    bool LoraModem::wasReceived () const
    {
        return received_;
    }

    uint16_t LoraModem::sender () const
    {
        return sender_;
    }

    const char* LoraModem::text () const
    {
        return text_;
    }

    int16_t LoraModem::signal () const
    {
        return signal_;
    }

    int8_t LoraModem::margin () const
    {
        return margin_;
    }

    void LoraModem::update (Millis now)
    {
        received_ = false;

        if (sending_ && sent_.elapsed (now) >= GiveUp)
        {
            sending_ = false;
        }

        while (reader_.read (port_))
        {
            const char* line = reader_.line ();

            if (strcmp (line, "+OK") == 0 || strncmp (line, "+ERR", 4) == 0)
            {
                sending_ = false;
            }
            else if (strncmp (line, "+RCV=", 5) == 0 && heard (line + 5))
            {
                received_ = true;
                return;
            }
        }
    }

    // Send a command and wait for the modem to say +OK. Anything else it
    // says meanwhile, such as +READY after starting up, is passed over.
    bool LoraModem::command (const char* line)
    {
        print (port_, line, "\r\n");
        unsigned long start = millis ();

        while (millis () - start < Answer)
        {
            if (!reader_.read (port_))
            {
                continue;
            }

            if (strcmp (reader_.line (), "+OK") == 0)
            {
                return true;
            }

            if (strncmp (reader_.line (), "+ERR", 4) == 0)
            {
                return false;
            }
        }

        return false;
    }

    // "address,length,text,signal,margin". The text may hold commas, so it
    // is taken by its length.
    bool LoraModem::heard (const char* fields)
    {
        char*       end  = nullptr;
        long        from = strtol (fields, &end, 10);
        const char* at   = end;

        if (*at != ',')
        {
            return false;
        }

        long length = strtol (at + 1, &end, 10);
        at          = end;

        if (*at != ',' || length < 0 || static_cast<size_t> (length) > strlen (at + 1))
        {
            return false;
        }

        const char* words = at + 1;
        at                = words + length;

        if (*at != ',')
        {
            return false;
        }

        long strength = strtol (at + 1, &end, 10);

        if (*end != ',')
        {
            return false;
        }

        long   above = strtol (end + 1, &end, 10);
        size_t kept  = min (static_cast<size_t> (length), static_cast<size_t> (MaxLength));

        memcpy (text_, words, kept);
        text_[kept] = '\0';
        sender_     = static_cast<uint16_t> (from);
        signal_     = static_cast<int16_t> (strength);
        margin_     = static_cast<int8_t> (above);
        return true;
    }
}
