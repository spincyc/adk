#include "fm_radio.h"

#include <Arduino.h>
#include <string.h>

namespace adk {

    namespace {

        constexpr uint8_t  Address  = 0x10;
        constexpr uint16_t Si4703Id = 0x1242;

        // The chip's registers. Reads start at StatusRssi and wrap round;
        // writes start at PowerConfig.
        constexpr uint8_t DeviceId    = 0x00;
        constexpr uint8_t PowerConfig = 0x02;
        constexpr uint8_t Channel     = 0x03;
        constexpr uint8_t SysConfig1  = 0x04;
        constexpr uint8_t SysConfig2  = 0x05;
        constexpr uint8_t SysConfig3  = 0x06;
        constexpr uint8_t Test1       = 0x07;
        constexpr uint8_t StatusRssi  = 0x0A;
        constexpr uint8_t ReadChannel = 0x0B;
        constexpr uint8_t RdsB        = 0x0D;
        constexpr uint8_t RdsC        = 0x0E;
        constexpr uint8_t RdsD        = 0x0F;
        constexpr uint8_t Everything  = 16;
        constexpr uint8_t News        = RdsD - StatusRssi + 1;
        constexpr uint8_t Status      = 2;

        constexpr uint16_t Unmute     = 1u << 14;    // PowerConfig: DMUTE
        constexpr uint16_t Verbose    = 1u << 11;    // RDSM: block errors reported
        constexpr uint16_t SeekUp     = 1u << 9;
        constexpr uint16_t Seek       = 1u << 8;
        constexpr uint16_t Enable     = 1u << 0;
        constexpr uint16_t Tune       = 1u << 15;    // Channel
        constexpr uint16_t Rds        = 1u << 12;    // SysConfig1
        constexpr uint16_t Europe     = 1u << 11;    // DE: 50 µs de-emphasis
        constexpr uint16_t Crystal    = 0x8100;      // Test1: XOSCEN, as AN230 gives it
        constexpr uint16_t RdsReady   = 1u << 15;    // StatusRssi
        constexpr uint16_t Complete   = 1u << 14;    // STC
        constexpr uint16_t Failed     = 1u << 13;    // SF/BL
        constexpr uint16_t Stereo     = 1u << 8;

        // AN230's recommended seek: a station must be 25 dBµV strong with a
        // signal-to-noise of 4 and at most 8 impulse counts.
        constexpr uint16_t SeekThreshold = 0x19u << 8;
        constexpr uint16_t SeekQuality   = 0x4u << 4 | 0x8u;

        constexpr uint8_t  InitialVolume = 8;
        constexpr uint8_t  Loudest       = 15;
        constexpr Millis   Settle        = 500;      // the crystal, after XOSCEN
        constexpr Millis   PowerUp       = 110;
        constexpr Millis   Listening     = 40;       // between reads of the news
        constexpr Millis   Tuning        = 60;       // the longest a tune takes
        constexpr Millis   Seeking       = 100;      // between checks on a seek
        constexpr Millis   Clearing      = 10;       // for STC to clear

        struct Band
        {
            uint16_t bottom;
            uint16_t top;
            uint8_t  spacing;
            uint16_t settings;                   // BAND and SPACE in SysConfig2
            uint16_t emphasis;
        };

        constexpr Band Bands [] = {
            {875, 1080, 1, 0x0u << 6 | 0x1u << 4, Europe},
            {875, 1080, 2, 0x0u << 6 | 0x0u << 4, 0},
            {760,  900, 1, 0x2u << 6 | 0x1u << 4, Europe}};

        const Band& bandOf (FmBand band)
        {
            return Bands[static_cast<uint8_t> (band)];
        }

        // RDS error counts: 0, 1-2 corrected, 3-5 corrected, too many.
        // Letters are only trusted with at most two corrected.
        bool trusted (uint8_t errors)
        {
            return errors <= 1;
        }

        char printable (uint8_t character)
        {
            return (character >= 0x20 && character < 0x7F) ? static_cast<char> (character) : ' ';
        }
    }

    FmRadio::FmRadio (Pin sdio, Pin sclk, Pin reset, FmBand band)
        : registers_ {}
        , name_      {}
        , nextName_  {}
        , text_      {}
        , polled_    (0)
        , wanted_    (0)
        , channel_   (0)
        , sdio_      (sdio)
        , sclk_      (sclk)
        , reset_     (reset)
        , band_      (band)
        , state_     (State::Off)
        , segments_  (0)
        , textFlag_  (0)
        , seek_      (0)
        , ok_        (false)
        , starting_  (false)
        , tuned_     (false)
        , named_     (false)
        , texted_    (false)
    {
    }

    void FmRadio::setup ()
    {
        state_ = State::Off;
        ok_    = false;

        if (!claimInput (sdio_) || !claimInput (sclk_) || !claimInput (reset_))
        {
            return;
        }

        // Pulling a pin low makes it an output; its level is set low once,
        // here, and is never set high, which would put 5 V on the chip.
        const Pin lines [] = {sdio_, sclk_, reset_};

        for (Pin pin : lines)
        {
            digitalWrite (pin, LOW);
        }

        // Holding SDIO low as RST rises chooses the two-wire bus.
        pull (reset_);
        pull (sdio_);
        delay (1);
        let (reset_);
        delay (1);
        let (sdio_);

        ok_ = read (Everything) && registers_[DeviceId] == Si4703Id;

        if (!ok_)
        {
            return;
        }

        registers_[Test1] = Crystal;
        write (Test1);
        delay (Settle);

        registers_[PowerConfig] = Unmute | Verbose | Enable;
        write (PowerConfig);
        delay (PowerUp);

        const Band& band = bandOf (band_);
        registers_[SysConfig1] = Rds | band.emphasis;
        registers_[SysConfig2] = SeekThreshold | band.settings | InitialVolume;
        registers_[SysConfig3] = SeekQuality;
        write (SysConfig3);

        state_ = State::Idle;
        startTune ();
    }

    bool FmRadio::ok () const
    {
        return ok_;
    }

    void FmRadio::tune (uint16_t frequency)
    {
        const Band& band = bandOf (band_);
        frequency        = constrain (frequency, band.bottom, band.top);
        wanted_          = static_cast<uint16_t> ((frequency - band.bottom) / band.spacing);

        if (state_ == State::Idle && wanted_ != channel_)
        {
            startTune ();
        }
    }

    void FmRadio::step (int8_t stations)
    {
        int32_t count = channels ();
        int32_t next  = (static_cast<int32_t> (wanted_) + stations) % count;
        wanted_       = static_cast<uint16_t> (next < 0 ? next + count : next);

        if (state_ == State::Idle && wanted_ != channel_)
        {
            startTune ();
        }
    }

    void FmRadio::seekUp ()
    {
        startSeek (true);
    }

    void FmRadio::seekDown ()
    {
        startSeek (false);
    }

    uint16_t FmRadio::frequency () const
    {
        const Band& band = bandOf (band_);
        return static_cast<uint16_t> (band.bottom + wanted_ * band.spacing);
    }

    bool FmRadio::isTuning () const
    {
        return state_ == State::Tuning || state_ == State::Settling;
    }

    bool FmRadio::wasTuned () const
    {
        return tuned_;
    }

    uint8_t FmRadio::signal () const
    {
        return static_cast<uint8_t> (registers_[StatusRssi] & 0xFF);
    }

    bool FmRadio::isStereo () const
    {
        return registers_[StatusRssi] & Stereo;
    }

    void FmRadio::setVolume (uint8_t volume)
    {
        uint16_t others = registers_[SysConfig2] & 0xFFF0;
        uint16_t loud   = static_cast<uint16_t> (others | min (volume, Loudest));

        if (loud == registers_[SysConfig2] && (registers_[PowerConfig] & Unmute))
        {
            return;
        }

        registers_[SysConfig2]   = loud;
        registers_[PowerConfig] |= Unmute;

        if (state_ != State::Off)
        {
            write (SysConfig2);
        }
    }

    uint8_t FmRadio::volume () const
    {
        return static_cast<uint8_t> (registers_[SysConfig2] & 0x0F);
    }

    const char* FmRadio::stationName () const
    {
        return name_;
    }

    const char* FmRadio::radioText () const
    {
        return text_;
    }

    bool FmRadio::nameChanged () const
    {
        return named_;
    }

    bool FmRadio::textChanged () const
    {
        return texted_;
    }

    void FmRadio::update (Millis now)
    {
        tuned_  = false;
        named_  = false;
        texted_ = false;

        if (state_ == State::Off)
        {
            return;
        }

        if (starting_)
        {
            polled_   = now;
            starting_ = false;
        }

        Millis wait = state_ == State::Idle     ? Listening
                    : state_ == State::Settling ? Clearing
                    : seek_ != 0                ? Seeking
                                                : Tuning;

        if (now - polled_ < wait)
        {
            return;
        }

        polled_ = now;

        if (state_ == State::Idle)
        {
            if (read (News))
            {
                readRds ();
            }

            return;
        }

        if (!read (Status))
        {
            return;
        }

        bool complete = registers_[StatusRssi] & Complete;

        // The chip says it has finished; it then waits for TUNE or SEEK to
        // be cleared, and clears STC in answer.
        if (state_ == State::Tuning && complete)
        {
            if (seek_ != 0 && !(registers_[StatusRssi] & Failed))
            {
                wanted_ = registers_[ReadChannel] & 0x3FF;
            }

            registers_[PowerConfig] &= static_cast<uint16_t> (~(Seek | SeekUp));
            registers_[Channel]     &= static_cast<uint16_t> (~Tune);
            write (Channel);
            state_ = State::Settling;
        }
        else if (state_ == State::Settling && !complete)
        {
            finishTune ();
        }
    }

    void FmRadio::stop ()
    {
        registers_[PowerConfig] &= static_cast<uint16_t> (~Unmute);

        if (state_ != State::Off)
        {
            write (PowerConfig);
        }
    }

    void FmRadio::startTune ()
    {
        clearRds ();
        registers_[Channel] = Tune | wanted_;
        write (Channel);
        seek_     = 0;
        starting_ = true;
        state_    = State::Tuning;
    }

    void FmRadio::startSeek (bool up)
    {
        if (state_ != State::Idle)
        {
            return;
        }

        clearRds ();
        registers_[PowerConfig] = static_cast<uint16_t> ((registers_[PowerConfig] & ~SeekUp)
                                                         | Seek | (up ? SeekUp : 0));
        write (PowerConfig);
        seek_     = up ? 1 : -1;
        starting_ = true;
        state_    = State::Tuning;
    }

    // Tuned. If the knob has moved on meanwhile, tune again.
    void FmRadio::finishTune ()
    {
        channel_ = registers_[ReadChannel] & 0x3FF;
        state_   = State::Idle;

        if (seek_ == 0 && channel_ != wanted_)
        {
            startTune ();
            return;
        }

        wanted_ = channel_;
        seek_   = 0;
        tuned_  = true;
    }

    // An RDS group is four 16-bit blocks. The second says which kind it is:
    // kind 0 carries two letters of the name, kind 2 two or four of the text.
    void FmRadio::readRds ()
    {
        if (!(registers_[StatusRssi] & RdsReady))
        {
            return;
        }

        uint16_t b      = registers_[RdsB];
        uint16_t c      = registers_[RdsC];
        uint16_t d      = registers_[RdsD];
        uint8_t  errorB = static_cast<uint8_t> (registers_[ReadChannel] >> 14 & 3);
        uint8_t  errorC = static_cast<uint8_t> (registers_[ReadChannel] >> 12 & 3);
        uint8_t  errorD = static_cast<uint8_t> (registers_[ReadChannel] >> 10 & 3);
        uint8_t  kind   = static_cast<uint8_t> (b >> 12);
        bool     versionB = b & 0x0800;

        if (!trusted (errorB))
        {
            return;
        }

        if (kind == 0 && trusted (errorD))
        {
            uint8_t segment                = b & 3;
            nextName_[2 * segment]         = printable (static_cast<uint8_t> (d >> 8));
            nextName_[2 * segment + 1]     = printable (static_cast<uint8_t> (d));
            segments_                     |= static_cast<uint8_t> (1 << segment);

            if (segments_ == 0x0F)
            {
                segments_ = 0;

                if (memcmp (name_, nextName_, sizeof nextName_) != 0)
                {
                    memcpy (name_, nextName_, sizeof nextName_);
                    named_ = true;
                }
            }
        }

        if (kind != 2)
        {
            return;
        }

        // A new text starts whenever the A/B flag flips.
        uint8_t flag = static_cast<uint8_t> (b >> 4 & 1);

        if (flag != textFlag_)
        {
            textFlag_ = flag;
            texted_   = text_[0] != '\0';
            memset (text_, 0, sizeof text_);
        }

        uint8_t letters [4] = {static_cast<uint8_t> (c >> 8), static_cast<uint8_t> (c),
                               static_cast<uint8_t> (d >> 8), static_cast<uint8_t> (d)};
        uint8_t count       = versionB ? 2 : 4;
        uint8_t first       = versionB ? 2 : 0;

        if (!trusted (errorD) || (!versionB && !trusted (errorC)))
        {
            return;
        }

        for (uint8_t index = 0; index < count; ++index)
        {
            uint8_t place  = static_cast<uint8_t> ((b & 0x0F) * count + index);
            uint8_t letter = letters[first + index];
            char    shown  = (letter == '\r') ? '\0' : printable (letter);

            if (text_[place] != shown)
            {
                text_[place] = shown;
                texted_      = true;
            }

            if (shown == '\0')
            {
                break;
            }
        }
    }

    void FmRadio::clearRds ()
    {
        memset (name_, 0, sizeof name_);
        memset (text_, 0, sizeof text_);
        segments_ = 0;
    }

    uint16_t FmRadio::channels () const
    {
        const Band& band = bandOf (band_);
        return static_cast<uint16_t> ((band.top - band.bottom) / band.spacing + 1);
    }

    // Read count registers, from StatusRssi on.
    bool FmRadio::read (uint8_t count)
    {
        if (!begin ())
        {
            return false;
        }

        bool answered = send (Address << 1 | 1);

        for (uint8_t index = 0; answered && index < count; ++index)
        {
            uint8_t high = take (true);
            uint8_t low  = take (index + 1 < count);

            registers_[(StatusRssi + index) & 0x0F] = static_cast<uint16_t> (high << 8 | low);
        }

        end ();
        return answered;
    }

    // Write the registers from PowerConfig to last.
    bool FmRadio::write (uint8_t last)
    {
        if (!begin ())
        {
            return false;
        }

        bool answered = send (Address << 1);

        for (uint8_t index = PowerConfig; answered && index <= last; ++index)
        {
            answered = send (static_cast<uint8_t> (registers_[index] >> 8))
                    && send (static_cast<uint8_t> (registers_[index]));
        }

        end ();
        return answered;
    }

    // A line is pulled low by making its pin an output, and let go by
    // making it an input again, when the 10 kΩ resistor takes it to 3.3 V.
    void FmRadio::pull (Pin pin)
    {
        pinMode (pin, OUTPUT);
        delayMicroseconds (2);
    }

    void FmRadio::let (Pin pin)
    {
        pinMode (pin, INPUT);
        delayMicroseconds (2);
    }

    // SDIO falling while SCLK is high starts a transfer; rising, ends it.
    bool FmRadio::begin ()
    {
        let (sdio_);
        let (sclk_);

        if (digitalRead (sdio_) == LOW || digitalRead (sclk_) == LOW)
        {
            return false;
        }

        pull (sdio_);
        pull (sclk_);
        return true;
    }

    void FmRadio::end ()
    {
        pull (sdio_);
        let  (sclk_);
        let  (sdio_);
    }

    // Eight bits, most significant first, each read while SCLK is high;
    // then the chip pulls SDIO low to say it heard.
    bool FmRadio::send (uint8_t byte)
    {
        for (uint8_t bit = 0x80; bit != 0; bit >>= 1)
        {
            if (byte & bit)
            {
                let (sdio_);
            }
            else
            {
                pull (sdio_);
            }

            let  (sclk_);
            pull (sclk_);
        }

        let (sdio_);
        let (sclk_);
        bool heard = digitalRead (sdio_) == LOW;
        pull (sclk_);
        return heard;
    }

    // Eight bits from the chip; then pull SDIO low if another byte is wanted.
    uint8_t FmRadio::take (bool more)
    {
        uint8_t byte = 0;
        let (sdio_);

        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            let (sclk_);
            byte = static_cast<uint8_t> (byte << 1 | (digitalRead (sdio_) == HIGH ? 1 : 0));
            pull (sclk_);
        }

        if (more)
        {
            pull (sdio_);
        }

        let  (sclk_);
        pull (sclk_);
        let  (sdio_);
        return byte;
    }
}
