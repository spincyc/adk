#include "check.h"
#include "fake_spi.h"

#include <adk/rfid.h>

#include <Arduino.h>

#include <algorithm>
#include <vector>

namespace {

    using Bytes = std::vector<uint8_t>;

    enum : uint8_t
    {
        CommandReg    = 0x01,
        ComIrqReg     = 0x04,
        ErrorReg      = 0x06,
        FifoDataReg   = 0x09,
        FifoLevelReg  = 0x0A,
        BitFramingReg = 0x0D,
        ModeReg       = 0x11,
        TxControlReg  = 0x14,
        TxAskReg      = 0x15,
        TModeReg      = 0x2A,
        TPrescalerReg = 0x2B,
        TReloadRegH   = 0x2C,
        TReloadRegL   = 0x2D,
        VersionReg    = 0x37
    };

    enum : uint8_t
    {
        Idle       = 0x00,
        Transmit   = 0x04,
        Transceive = 0x0C,
        SoftReset  = 0x0F
    };

    const uint8_t Reqa = 0x26;
    const uint8_t Wupa = 0x52;
    const Bytes   Anticollision {0x93, 0x20};
    const Bytes   Halt          {0x50, 0x00, 0x57, 0xCD};
    const Bytes   Atqa          {0x04, 0x00};

    // ISO 14443-A: a tag starts idle, a request or wake-up makes it ready,
    // and anything out of turn while it is ready drops it back to idle.
    // Only a wake-up reaches a halted tag.
    enum class Tag
    {
        Idle,
        Ready,
        Halted
    };

    // An MFRC522 as its registers and FIFO, with a tag a test can put in its
    // field and take away. Its radio is instant: the tag's answer is in the
    // FIFO as soon as the frame is sent.
    struct Rc522 : fake::SpiChip
    {
        explicit Rc522 (adk::Pin select, uint8_t chipVersion = 0x92)
            : SpiChip (select)
            , version (chipVersion)
        {
            powerUp ();
        }

        void present (uint32_t id)
        {
            uid     = {uint8_t (id >> 24), uint8_t (id >> 16), uint8_t (id >> 8), uint8_t (id)};
            uid.push_back (uid[0] ^ uid[1] ^ uid[2] ^ uid[3]);
            here    = true;
            tag     = Tag::Idle;
        }

        void remove ()
        {
            here = false;
        }

        int sent (const Bytes& frame) const
        {
            return static_cast<int> (std::count (frames.begin (), frames.end (), frame));
        }

        uint8_t exchange (uint8_t byte) override
        {
            if (transferred++ == 0)
            {
                ++transactions;
                badAddresses += byte & 0x01;
                reading       = byte & 0x80;
                address       = (byte >> 1) & 0x3F;
                return 0;
            }

            if (reading)
            {
                uint8_t value = read (address);
                address       = (byte >> 1) & 0x3F;
                return value;
            }

            write (address, byte);
            return 0;
        }

        void deselected () override
        {
            oddTransactions += (transferred != 0 && transferred != 2) ? 1 : 0;
            transferred      = 0;
        }

        void powerUp ()
        {
            memset (registers, 0, sizeof registers);
            registers[CommandReg]   = 0x20;
            registers[ComIrqReg]    = 0x14;
            registers[ModeReg]      = 0x3F;
            registers[TxControlReg] = 0x80;
            fifo.clear ();
        }

        uint8_t read (uint8_t reg)
        {
            switch (reg)
            {
                case FifoLevelReg: return static_cast<uint8_t> (fifo.size ());
                case VersionReg:   return version;
                case FifoDataReg:
                {
                    uint8_t byte = fifo.empty () ? 0 : fifo.front ();
                    fifo.erase (fifo.begin (), fifo.begin () + (fifo.empty () ? 0 : 1));
                    return byte;
                }
                default:           return registers[reg];
            }
        }

        void write (uint8_t reg, uint8_t value)
        {
            switch (reg)
            {
                case CommandReg:
                    registers[CommandReg] = value;
                    command (value & 0x0F);
                    break;

                case ComIrqReg:
                    // Bit 7 says whether the marked bits are set or cleared.
                    registers[ComIrqReg] = (value & 0x80)
                        ? static_cast<uint8_t> (registers[ComIrqReg] | (value & 0x7F))
                        : static_cast<uint8_t> (registers[ComIrqReg] & ~value);
                    break;

                case FifoDataReg:
                    fifo.push_back (value);
                    break;

                case FifoLevelReg:
                    fifo.clear ();
                    break;

                case BitFramingReg:
                    registers[BitFramingReg] = value & 0x7F;

                    if ((value & 0x80) && (registers[CommandReg] & 0x0F) == Transceive)
                    {
                        air (true);
                    }
                    break;

                case TModeReg:
                    registers[TModeReg] = value;
                    configuredAt        = arduino::now ();
                    break;

                default:
                    registers[reg] = value;
                    break;
            }
        }

        void command (uint8_t code)
        {
            if (code == SoftReset)
            {
                powerUp ();
                ++softResets;
                resetAt = arduino::now ();
            }
            else if (code == Transmit)
            {
                air (false);
                registers[CommandReg] &= 0xF0;
                registers[ComIrqReg]  |= 0x50;
            }
        }

        // Send the FIFO to the tag, and if listening, put its answer back.
        void air (bool listening)
        {
            Bytes frame = fifo;
            fifo.clear ();
            frames.push_back (frame);

            Bytes answer = hear (frame, registers[BitFramingReg] & 0x07);

            if (!listening || stuck)
            {
                return;
            }

            if (answer.empty ())
            {
                registers[ComIrqReg] |= 0x41;
                registers[ErrorReg]   = 0;
                return;
            }

            fifo                  = answer;
            registers[ComIrqReg] |= 0x60;
            registers[ErrorReg]   = garbled > 0 ? 0x02 : 0;
            garbled              -= garbled > 0 ? 1 : 0;
        }

        Bytes hear (const Bytes& frame, uint8_t lastBits)
        {
            if (!here || (registers[TxControlReg] & 0x03) != 0x03)
            {
                tag = Tag::Idle;
                return {};
            }

            if (frame.size () == 1 && lastBits == 7 && (frame[0] == Reqa || frame[0] == Wupa))
            {
                bool wakes = tag == Tag::Idle || (frame[0] == Wupa && tag == Tag::Halted);
                tag        = wakes ? Tag::Ready : (tag == Tag::Ready ? Tag::Idle : tag);
                return wakes ? Atqa : Bytes {};
            }

            if (tag == Tag::Ready && frame == Anticollision && lastBits == 0)
            {
                return uid;
            }

            // Strictly a ready tag goes idle on HLTA; some go to halt. Halt is
            // the harder case: only WUPA wakes it.
            if (tag == Tag::Ready)
            {
                tag = (frame == Halt) ? Tag::Halted : Tag::Idle;
            }

            return {};
        }

        uint8_t registers [64];
        Bytes   fifo;
        uint8_t version;
        bool    stuck   = false;
        int     garbled = 0;

        bool  here = false;
        Bytes uid;
        Tag   tag  = Tag::Idle;

        int     transferred = 0;
        uint8_t address     = 0;
        bool    reading     = false;

        std::vector<Bytes> frames;
        int                transactions    = 0;
        int                oddTransactions = 0;
        int                badAddresses    = 0;
        int                softResets      = 0;
        unsigned long      resetAt         = 0;
        unsigned long      configuredAt    = 0;
    };

    // Update once a millisecond over [from, to) and count the reads.
    int readsBetween (const adk::Rfid& rfid, adk::Millis from, adk::Millis to)
    {
        int reads = 0;

        for (adk::Millis now = from; now < to; ++now)
        {
            adk::update (now);
            reads += rfid.wasRead () ? 1 : 0;
        }

        return reads;
    }
}

TEST (rfidClaimsTheBusAndStartsTheReader)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (rfid.ok ());
    CHECK (arduino::pin (9).mode == OUTPUT);
    CHECK (arduino::pin (9).output == HIGH);
    CHECK (arduino::pin (8).mode == OUTPUT);
    CHECK (arduino::pin (8).output == HIGH);
    CHECK (arduino::pin (MISO).mode == INPUT);
    CHECK (arduino::pin (MOSI).mode == OUTPUT);
    CHECK (arduino::pin (SCK).mode == OUTPUT);
    CHECK (arduino::pin (SS).mode == OUTPUT);
    CHECK (arduino::pin (SS).output == HIGH);

    CHECK (reader.softResets == 1);
    CHECK (reader.configuredAt - reader.resetAt >= 50000);
    CHECK (reader.registers[TModeReg] == 0x80);
    CHECK (reader.registers[TPrescalerReg] == 0xA9);
    CHECK (reader.registers[TReloadRegH] == 0x03);
    CHECK (reader.registers[TReloadRegL] == 0xE8);
    CHECK (reader.registers[TxAskReg] == 0x40);
    CHECK (reader.registers[ModeReg] == 0x3D);
    CHECK (reader.registers[TxControlReg] == 0x83);
}

TEST (rfidBusPinsCannotBeUsedForAnythingElse)
{
    adk::Led  led    {52};
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::PinInUse);
    CHECK (adk::faultPin () == 52);
}

TEST (rfidCanUsePin53AsItsSelect)
{
    Rc522     reader {53};
    adk::Rfid rfid   {53, 8};

    CHECK (adk::start ());
    reader.present (0x1A2B3C4D);

    CHECK (readsBetween (rfid, 0, 10) == 1);
    CHECK (arduino::pin (53).output == HIGH);
}

TEST (rfidAcceptsCloneChipsButNotSilence)
{
    {
        Rc522     clone {9, 0x88};
        adk::Rfid rfid  {9, 8};

        adk::setup ();
        CHECK (rfid.ok ());
    }

    {
        Rc522     dead {9, 0x00};
        adk::Rfid rfid {9, 8};

        adk::setup ();
        CHECK (!rfid.ok ());

        dead.present (0x1A2B3C4D);
        CHECK (readsBetween (rfid, 0, 1000) == 0);
        CHECK (dead.frames.empty ());
    }

    {
        adk::Rfid rfid {9, 8};

        adk::setup ();
        CHECK (!rfid.ok ());

        int bytes = fake::spiLog.bytes;
        adk::update (1000);
        adk::stop ();
        CHECK (fake::spiLog.bytes == bytes);
    }
}

TEST (rfidReadsATagOnceWhileItStays)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);

    CHECK (readsBetween (rfid, 0, 10) == 1);
    CHECK (rfid.isPresent ());
    CHECK (rfid.uid () == 0x1A2B3C4D);

    CHECK (readsBetween (rfid, 10, 3000) == 0);
    CHECK (rfid.isPresent ());
    CHECK (reader.sent (Anticollision) == 30);
    CHECK (reader.sent (Halt) == 30);
}

TEST (rfidForgetsARemovedTagAndReadsItAgain)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 0, 10) == 1);

    reader.remove ();
    CHECK (readsBetween (rfid, 10, 150) == 0);
    CHECK (rfid.isPresent ());

    CHECK (readsBetween (rfid, 150, 250) == 0);
    CHECK (!rfid.isPresent ());
    CHECK (rfid.uid () == 0x1A2B3C4D);

    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 250, 400) == 1);
    CHECK (rfid.isPresent ());
}

TEST (rfidRidesOutOneMissedLook)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 0, 50) == 1);

    reader.remove ();
    CHECK (readsBetween (rfid, 50, 150) == 0);

    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 150, 1000) == 0);
    CHECK (rfid.isPresent ());
}

TEST (rfidReadsADifferentTagAsANewArrival)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 0, 50) == 1);

    reader.present (0xDEADBEEF);
    CHECK (readsBetween (rfid, 50, 150) == 1);
    CHECK (rfid.uid () == 0xDEADBEEF);
}

TEST (rfidWithNoTagOnlyLooksEvery100ms)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();

    CHECK (readsBetween (rfid, 0, 1000) == 0);
    CHECK (!rfid.isPresent ());
    CHECK (rfid.uid () == 0);
    CHECK (reader.sent ({Wupa}) == 10);
    CHECK (reader.sent (Anticollision) == 0);
    CHECK (reader.sent (Halt) == 10);
    CHECK (reader.sent ({Reqa}) == 0);
}

TEST (rfidIgnoresATagWithABadCheckByte)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    reader.uid[4] ^= 0x01;

    CHECK (readsBetween (rfid, 0, 1000) == 0);
    CHECK (!rfid.isPresent ());
    CHECK (rfid.uid () == 0);
    CHECK (reader.sent (Anticollision) == 10);
}

TEST (rfidIgnoresSevenByteUids)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x88040A0B);

    CHECK (readsBetween (rfid, 0, 1000) == 0);
    CHECK (!rfid.isPresent ());
}

TEST (rfidIgnoresGarbledAnswers)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    reader.garbled = 1000;

    CHECK (readsBetween (rfid, 0, 1000) == 0);
    CHECK (!rfid.isPresent ());
}

TEST (rfidRidesOutOneGarbledAnswer)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 0, 50) == 1);

    reader.garbled = 1;
    CHECK (readsBetween (rfid, 50, 1000) == 0);
    CHECK (rfid.isPresent ());
}

TEST (rfidGivesUpOnAnExchangeThatNeverEnds)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    reader.stuck = true;

    CHECK (readsBetween (rfid, 0, 1000) == 0);
    CHECK (reader.sent ({Wupa}) == 10);
}

TEST (rfidSelectsTheReaderForEachAccessOnly)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);

    bool selectedBetweenUpdates = false;

    for (adk::Millis now = 0; now < 500; ++now)
    {
        adk::update (now);
        selectedBetweenUpdates |= arduino::pin (9).output == LOW;
    }

    CHECK (!selectedBetweenUpdates);
    CHECK (reader.transactions > 100);
    CHECK (reader.oddTransactions == 0);
    CHECK (reader.badAddresses == 0);
    CHECK (fake::spiLog.strays == 0);
    CHECK (fake::spiLog.clashes == 0);
}

TEST (twoReadersShareTheBus)
{
    Rc522     front     {9};
    Rc522     back      {10};
    adk::Rfid frontRfid {9, 8};
    adk::Rfid backRfid  {10, 7};

    CHECK (adk::start ());
    front.present (0x11223344);
    back.present (0x55667788);

    readsBetween (frontRfid, 0, 10);

    CHECK (frontRfid.uid () == 0x11223344);
    CHECK (backRfid.uid () == 0x55667788);
    CHECK (fake::spiLog.begins == 2);
    CHECK (fake::spiLog.strays == 0);
    CHECK (fake::spiLog.clashes == 0);
}

TEST (rfidStopSwitchesTheFieldOff)
{
    Rc522     reader {9};
    adk::Rfid rfid   {9, 8};

    adk::setup ();
    reader.present (0x1A2B3C4D);
    CHECK (readsBetween (rfid, 0, 10) == 1);

    adk::stop ();
    CHECK ((reader.registers[TxControlReg] & 0x03) == 0);
    CHECK (!rfid.isPresent ());

    size_t frames = reader.frames.size ();
    CHECK (readsBetween (rfid, 10, 1000) == 0);
    CHECK (reader.frames.size () == frames);
}
