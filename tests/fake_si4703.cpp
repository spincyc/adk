#include "fake_si4703.h"

#include <Arduino.h>

namespace fake {

    namespace {

        const uint8_t Address = 0x10;

        const uint8_t PowerConfig = 0x02;
        const uint8_t Channel     = 0x03;
        const uint8_t StatusRssi  = 0x0A;
        const uint8_t ReadChannel = 0x0B;
        const uint8_t RdsA        = 0x0C;

        const uint16_t SeekUp   = 1u << 9;
        const uint16_t Seek     = 1u << 8;
        const uint16_t Tune     = 1u << 15;
        const uint16_t RdsReady = 1u << 15;
        const uint16_t Complete = 1u << 14;
        const uint16_t Failed   = 1u << 13;
        const uint16_t Stereo   = 1u << 8;

        const uint8_t Hiss = 8;         // dBµV between stations
    }

    Si4703::Si4703 (adk::Pin sdio, adk::Pin sclk, adk::Pin reset)
        : registers  {}
        , tuneMicros (60000)
        , twoWire    (false)
        , drivenHigh (false)
        , sdio_      (sdio)
        , sclk_      (sclk)
        , reset_     (reset)
        , phase_     (Phase::Idle)
        , next_      (Phase::Idle)
        , clocks_    (0)
        , shift_     (0)
        , index_     (0)
        , high_      (0)
        , byte_      (0)
        , holding_   (false)
        , ack_       (false)
        , scl_       (true)
        , sda_       (true)
        , rst_       (true)
        , busy_      (false)
        , doneAt_    (0)
        , result_    (0)
        , failed_    (false)
    {
        registers[0x00] = 0x1242;
        registers[0x01] = 0x1253;
        registers[0x07] = 0x0100;

        arduino::onPinMode = [this] (uint8_t pin, uint8_t mode)
        {
            if (!mine (pin))
            {
                return;
            }

            if (mode == OUTPUT && arduino::pin (pin).output == HIGH)
            {
                drivenHigh = true;
            }

            change ();
        };

        // A high output drives 5 V; a high input turns on the pull-up to 5 V.
        arduino::onDigitalWrite = [this] (uint8_t pin, uint8_t value)
        {
            if (mine (pin) && value == HIGH)
            {
                drivenHigh = true;
            }
        };

        arduino::onDigitalRead = [this] (uint8_t pin)
        {
            if (pin == sdio_)
            {
                return sda_ ? HIGH : LOW;
            }

            if (pin == sclk_)
            {
                return scl_ ? HIGH : LOW;
            }

            return static_cast<int> (arduino::pin (pin).input);
        };
    }

    Si4703::~Si4703 ()
    {
        arduino::onPinMode      = nullptr;
        arduino::onDigitalWrite = nullptr;
        arduino::onDigitalRead  = nullptr;
    }

    bool Si4703::mine (uint8_t pin) const
    {
        return pin == sdio_ || pin == sclk_ || pin == reset_;
    }

    bool Si4703::pulled (uint8_t pin) const
    {
        return arduino::pin (pin).mode == OUTPUT;
    }

    // Work out the lines' levels after a pin change, and what the change
    // means: a start or stop while SCLK is high, otherwise a clock edge.
    void Si4703::change ()
    {
        bool rst = !pulled (reset_);
        bool scl = !pulled (sclk_);

        if (!rst)
        {
            twoWire  = false;
            phase_   = Phase::Idle;
            holding_ = false;
        }

        bool sda = !(pulled (sdio_) || holding_);

        if (rst && !rst_)
        {
            twoWire = !sda;
        }

        bool wasScl = scl_;
        bool wasSda = sda_;
        rst_        = rst;
        scl_        = scl;
        sda_        = sda;

        if (!rst || !twoWire)
        {
            return;
        }

        if (scl && wasScl && wasSda && !sda)
        {
            phase_   = Phase::Address;
            clocks_  = 0;
            shift_   = 0;
            holding_ = false;
        }
        else if (scl && wasScl && !wasSda && sda)
        {
            phase_   = Phase::Idle;
            holding_ = false;
        }
        else if (scl && !wasScl)
        {
            rise ();
        }
        else if (!scl && wasScl)
        {
            fall ();
        }

        sda_ = !(pulled (sdio_) || holding_);
    }

    void Si4703::rise ()
    {
        ++clocks_;

        if ((phase_ == Phase::Address || phase_ == Phase::Write) && clocks_ <= 8)
        {
            shift_ = static_cast<uint8_t> (shift_ << 1 | (sda_ ? 1 : 0));
        }
        else if (phase_ == Phase::Read && clocks_ == 9)
        {
            ack_ = !sda_;
        }
    }

    void Si4703::fall ()
    {
        if (phase_ == Phase::Address || phase_ == Phase::Write)
        {
            if (clocks_ == 8)
            {
                heard (shift_);
            }
            else if (clocks_ == 9)
            {
                holding_ = false;
                clocks_  = 0;
                shift_   = 0;

                if (phase_ == Phase::Address)
                {
                    phase_ = next_;

                    if (phase_ == Phase::Read)
                    {
                        startRead ();
                        loadByte ();
                        holding_ = !(byte_ & 0x80);
                    }
                }
            }
        }
        else if (phase_ == Phase::Read)
        {
            if (clocks_ >= 1 && clocks_ <= 7)
            {
                holding_ = !((byte_ >> (7 - clocks_)) & 1);
            }
            else if (clocks_ == 8)
            {
                holding_ = false;
            }
            else if (clocks_ == 9)
            {
                clocks_ = 0;

                if (ack_)
                {
                    loadByte ();
                    holding_ = !(byte_ & 0x80);
                }
                else
                {
                    phase_   = Phase::Ignore;
                    holding_ = false;
                }
            }
        }
    }

    // A whole byte arrived: the address, or two to a register from 02h on.
    void Si4703::heard (uint8_t byte)
    {
        if (phase_ == Phase::Address)
        {
            if ((byte >> 1) != Address)
            {
                phase_ = Phase::Ignore;
                return;
            }

            next_    = (byte & 1) ? Phase::Read : Phase::Write;
            index_   = 0;
            holding_ = true;
            return;
        }

        uint8_t reg = static_cast<uint8_t> ((PowerConfig + index_ / 2) & 0x0F);

        if (index_ % 2 == 0)
        {
            high_ = byte;
        }
        else
        {
            uint16_t before = registers[reg];
            registers[reg]  = static_cast<uint16_t> (high_ << 8 | byte);
            written (reg, before);
        }

        ++index_;
        holding_ = true;
    }

    // Setting TUNE or SEEK starts one; clearing it after STC clears STC.
    void Si4703::written (uint8_t reg, uint16_t before)
    {
        uint16_t now  = registers[reg];
        uint16_t flag = (reg == Channel) ? Tune : (reg == PowerConfig) ? Seek : 0;

        if (flag == 0 || (now & flag) == (before & flag))
        {
            return;
        }

        if (!(now & flag))
        {
            registers[StatusRssi] &= static_cast<uint16_t> (~(Complete | Failed));
            return;
        }

        uint16_t from = registers[ReadChannel] & 0x3FF;
        failed_       = false;
        result_       = static_cast<uint16_t> (registers[Channel] & 0x3FF);

        if (reg == PowerConfig)
        {
            bool     up    = now & SeekUp;
            bool     found = false;
            uint16_t best  = from;

            for (const Station& station : stations)
            {
                uint16_t ahead = static_cast<uint16_t> (up ? station.channel - from
                                                           : from - station.channel) & 0x3FF;
                uint16_t sofar = static_cast<uint16_t> (up ? best - from : from - best) & 0x3FF;

                if (station.channel != from && (!found || ahead < sofar))
                {
                    best  = station.channel;
                    found = true;
                }
            }

            result_ = best;
            failed_ = !found;
        }

        busy_   = true;
        doneAt_ = arduino::now () + tuneMicros;
    }

    void Si4703::settle ()
    {
        if (!busy_ || arduino::now () < doneAt_)
        {
            return;
        }

        busy_ = false;

        uint16_t status = static_cast<uint16_t> (Complete | strength (result_));

        if (failed_)
        {
            status |= Failed;
        }

        for (const Station& station : stations)
        {
            if (station.channel == result_ && station.stereo)
            {
                status |= Stereo;
            }
        }

        registers[StatusRssi]  = status;
        registers[ReadChannel] = result_;
    }

    // A read starts at 0Ah. When nothing is tuning, each read carries the
    // next RDS group, if there is one.
    void Si4703::startRead ()
    {
        settle ();
        index_                 = 0;
        registers[StatusRssi] &= static_cast<uint16_t> (~RdsReady);

        if (busy_ || (registers[StatusRssi] & Complete) || groups.empty ())
        {
            return;
        }

        const Group& group = groups.front ();
        registers[RdsA]     = group.a;
        registers[RdsA + 1] = group.b;
        registers[RdsA + 2] = group.c;
        registers[RdsA + 3] = group.d;

        registers[ReadChannel] = static_cast<uint16_t> ((registers[ReadChannel] & 0x3FF)
                                                        | group.errorsB << 14 | group.errorsC << 12
                                                        | group.errorsD << 10);
        registers[StatusRssi] |= RdsReady;
        groups.erase (groups.begin ());
    }

    void Si4703::loadByte ()
    {
        uint16_t value = registers[(StatusRssi + index_ / 2) & 0x0F];
        byte_          = static_cast<uint8_t> (index_ % 2 == 0 ? value >> 8 : value);
        ++index_;
    }

    uint8_t Si4703::strength (uint16_t channel) const
    {
        for (const Station& station : stations)
        {
            if (station.channel == channel)
            {
                return station.strength;
            }
        }

        return Hiss;
    }
}
