#include "rfid.h"

#include "spi.h"

#include <Arduino.h>

namespace adk {

    namespace {

        // MFRC522 registers.
        constexpr uint8_t CommandReg    = 0x01;
        constexpr uint8_t ComIrqReg     = 0x04;
        constexpr uint8_t ErrorReg      = 0x06;
        constexpr uint8_t FifoDataReg   = 0x09;
        constexpr uint8_t FifoLevelReg  = 0x0A;
        constexpr uint8_t BitFramingReg = 0x0D;
        constexpr uint8_t ModeReg       = 0x11;
        constexpr uint8_t TxControlReg  = 0x14;
        constexpr uint8_t TxAskReg      = 0x15;
        constexpr uint8_t TModeReg      = 0x2A;
        constexpr uint8_t TPrescalerReg = 0x2B;
        constexpr uint8_t TReloadRegH   = 0x2C;
        constexpr uint8_t TReloadRegL   = 0x2D;
        constexpr uint8_t VersionReg    = 0x37;

        // Its commands, and the register bits this driver uses.
        constexpr uint8_t Idle       = 0x00;
        constexpr uint8_t Transmit   = 0x04;
        constexpr uint8_t Transceive = 0x0C;
        constexpr uint8_t SoftReset  = 0x0F;

        constexpr uint8_t StartSend   = 0x80;   // BitFramingReg
        constexpr uint8_t FlushBuffer = 0x80;   // FifoLevelReg
        constexpr uint8_t FifoLevel   = 0x7F;   // FifoLevelReg
        constexpr uint8_t AntennaOn   = 0x03;   // TxControlReg: Tx1RFEn, Tx2RFEn
        constexpr uint8_t AllIrqs     = 0x7F;   // ComIrqReg, written to clear them
        constexpr uint8_t Received    = 0x30;   // ComIrqReg: RxIRq, IdleIRq
        constexpr uint8_t TimedOut    = 0x01;   // ComIrqReg: TimerIRq
        constexpr uint8_t Garbled     = 0x1B;   // ErrorReg: overflow, collision, parity, protocol

        // ISO 14443-A frames. WUPA wakes a tag whether it is idle or halted,
        // the anticollision command asks it for its UID, and HLTA (with its
        // CRC) halts it.
        constexpr uint8_t Wupa []          PROGMEM = {0x52};
        constexpr uint8_t Anticollision [] PROGMEM = {0x93, 0x20};
        constexpr uint8_t Halt []          PROGMEM = {0x50, 0x00, 0x57, 0xCD};

        // The first byte of a 7-byte UID's first part.
        constexpr uint8_t CascadeTag = 0x88;

        constexpr Millis  LookPeriod = 100;
        constexpr uint8_t Pending    = 0xFF;
    }

    Rfid::Rfid (Pin select, Pin reset)
        : lookedAt_ (0)
        , uid_      (0)
        , select_   (select)
        , reset_    (reset)
        , step_     (Step::Off)
        , misses_   (0)
        , ok_       (false)
        , present_  (false)
        , wasRead_  (false)
    {
    }

    void Rfid::setup ()
    {
        step_ = Step::Off;
        ok_   = false;

        bool claimed = spi::begin (select_) && claimOutput (reset_, true);

        if (!claimed)
        {
            return;
        }

        // Raising RST wakes the chip from power-down and SoftReset restarts
        // it. The datasheet gives no time for either; 50 ms is ample.
        writeRegister (CommandReg, SoftReset);
        delay (50);

        // Genuine chips report 0x91 or 0x92 and clones other values. With no
        // chip there, MISO reads all zeros or all ones.
        uint8_t version = readRegister (VersionReg);
        ok_ = version != 0x00 && version != 0xFF;

        if (!ok_)
        {
            return;
        }

        // Give up on a tag 25 ms after sending to it (1000 ticks of 13.56 MHz
        // / 339), modulate fully as ISO 14443-A requires, preset the CRC to
        // 0x6363 as it specifies, and switch the field on.
        writeRegister (TModeReg,      0x80);
        writeRegister (TPrescalerReg, 0xA9);
        writeRegister (TReloadRegH,   0x03);
        writeRegister (TReloadRegL,   0xE8);
        writeRegister (TxAskReg,      0x40);
        writeRegister (ModeReg,       0x3D);
        field (true);

        step_ = Step::Due;
    }

    bool Rfid::wasRead () const
    {
        return wasRead_;
    }

    bool Rfid::isPresent () const
    {
        return present_;
    }

    uint32_t Rfid::uid () const
    {
        return uid_;
    }

    bool Rfid::ok () const
    {
        return ok_;
    }

    // The reader does the radio work itself, so update () only starts an
    // exchange with a tag, and in later updates collects the answer.
    void Rfid::update (Millis now)
    {
        wasRead_ = false;

        switch (step_)
        {
            case Step::Off:
                break;

            case Step::Due:
                look (now);
                break;

            case Step::Resting:
                if (now - lookedAt_ >= LookPeriod)
                {
                    look (now);
                }
                break;

            case Step::Waking:
            case Step::Listing:
                listen (now);
                break;
        }
    }

    void Rfid::stop ()
    {
        // Before setup (), or with no reader, there is no field to switch off.
        if (step_ == Step::Off)
        {
            return;
        }

        writeRegister (CommandReg, Idle);
        field (false);

        step_    = Step::Off;
        present_ = false;
    }

    void Rfid::look (Millis now)
    {
        lookedAt_ = now;
        step_     = Step::Waking;
        send (Wupa, sizeof Wupa, 7, Transceive);
    }

    void Rfid::listen (Millis now)
    {
        uint8_t bytes [5];
        uint8_t length = reply (bytes, sizeof bytes);

        // The reader's timer ends every exchange within 25 ms. One still
        // going when the next look is due has gone wrong, and counts as silence.
        if (length == Pending && now - lookedAt_ < LookPeriod)
        {
            return;
        }

        // Any tag answers WUPA with two bytes (its ATQA), then the
        // anticollision command with its UID and their XOR as a check byte.
        if (step_ == Step::Waking && length == 2)
        {
            step_ = Step::Listing;
            send (Anticollision, sizeof Anticollision, 0, Transceive);
            return;
        }

        if (step_ == Step::Listing && length == 5 && bytes[0] != CascadeTag
            && (bytes[0] ^ bytes[1] ^ bytes[2] ^ bytes[3]) == bytes[4])
        {
            found (static_cast<uint32_t> (bytes[0]) << 24 | static_cast<uint32_t> (bytes[1]) << 16
                   | static_cast<uint32_t> (bytes[2]) << 8 | bytes[3]);
        }
        else
        {
            missed ();
        }

        // A tag that has answered, even garbled, waits for the command that
        // would select it, and takes anything else, even the next WUPA, as a
        // reason to fall silent. Halting it lets the next WUPA wake it.
        send (Halt, sizeof Halt, 0, Transmit);
        step_ = Step::Resting;
    }

    void Rfid::found (uint32_t uid)
    {
        wasRead_ = !present_ || uid != uid_;
        uid_     = uid;
        present_ = true;
        misses_  = 0;
    }

    void Rfid::missed ()
    {
        // One silent look can be a tag at the edge of the field; two in a
        // row mean it has gone.
        if (present_ && ++misses_ >= 2)
        {
            present_ = false;
        }
    }

    void Rfid::field (bool on)
    {
        uint8_t control = readRegister (TxControlReg);
        control         = static_cast<uint8_t> (on ? control | AntennaOn : control & ~AntennaOn);
        writeRegister (TxControlReg, control);
    }

    // Load a frame into the reader and start sending it. lastBits is how many
    // bits of the last byte go out, with 0 meaning all eight.
    void Rfid::send (const uint8_t* frame, uint8_t length, uint8_t lastBits, uint8_t command)
    {
        writeRegister (CommandReg,   Idle);
        writeRegister (ComIrqReg,    AllIrqs);
        writeRegister (FifoLevelReg, FlushBuffer);

        for (uint8_t index = 0; index < length; ++index)
        {
            writeRegister (FifoDataReg, pgm_read_byte (frame + index));
        }

        writeRegister (BitFramingReg, lastBits);
        writeRegister (CommandReg,    command);

        if (command == Transceive)
        {
            writeRegister (BitFramingReg, static_cast<uint8_t> (StartSend | lastBits));
        }
    }

    // How many bytes a tag answered with, or Pending while the reader is
    // still listening. A timeout, a garbled answer, and two tags answering at
    // once all count as no answer.
    uint8_t Rfid::reply (uint8_t* bytes, uint8_t capacity)
    {
        uint8_t irqs = readRegister (ComIrqReg);

        if (!(irqs & Received))
        {
            return (irqs & TimedOut) ? 0 : Pending;
        }

        uint8_t length = readRegister (FifoLevelReg) & FifoLevel;

        if ((readRegister (ErrorReg) & Garbled) || length > capacity)
        {
            return 0;
        }

        for (uint8_t index = 0; index < length; ++index)
        {
            bytes[index] = readRegister (FifoDataReg);
        }

        return length;
    }

    // One SPI transaction per access: the address shifted left one bit, with
    // bit 7 set to read, then the data.
    uint8_t Rfid::readRegister (uint8_t address)
    {
        digitalWrite  (select_, LOW);
        spi::transfer (static_cast<uint8_t> (0x80 | ((address << 1) & 0x7E)));
        uint8_t value = spi::transfer (0);
        digitalWrite  (select_, HIGH);
        return value;
    }

    void Rfid::writeRegister (uint8_t address, uint8_t value)
    {
        digitalWrite  (select_, LOW);
        spi::transfer (static_cast<uint8_t> ((address << 1) & 0x7E));
        spi::transfer (value);
        digitalWrite  (select_, HIGH);
    }
}
