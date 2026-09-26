#include "check.h"
#include "fake_spi.h"

#include <adk/rfid.h>
#include <adk/spi.h>

#include <Arduino.h>

// These drive src/adk/spi.cpp against the fake core's model of the SPI
// unit, with chips from fake_spi.h on the far end of the wires.

namespace {

    // A chip that answers each byte with the one before it, and keeps
    // everything it was sent.
    struct Echo : fake::SpiChip
    {
        explicit Echo (adk::Pin select)
            : SpiChip (select)
        {
        }

        uint8_t exchange (uint8_t byte) override
        {
            uint8_t answer = last;

            heard += static_cast<char> (byte);
            last   = byte;
            return answer;
        }

        std::string heard;
        uint8_t     last = 0x00;
    };

    void select (adk::Pin pin, bool selected)
    {
        digitalWrite (pin, selected ? LOW : HIGH);
    }
}

TEST (spiBeginsAsMasterInMode0MsbFirstAt4MHz)
{
    CHECK (adk::spi::begin (49));

    // SPE and MSTR only: no interrupt, most significant bit first, clock
    // idle low and sampled on its rising edge, 16 MHz / 4.
    CHECK (SPCR == (_BV (SPE) | _BV (MSTR)));
    CHECK ((SPSR & _BV (SPI2X)) == 0);

    CHECK (arduino::pin (SCK).mode == OUTPUT);
    CHECK (arduino::pin (MOSI).mode == OUTPUT);
    CHECK (arduino::pin (MISO).mode == INPUT);
    CHECK (arduino::pin (49).mode == OUTPUT);
    CHECK (arduino::pin (49).output == HIGH);
}

TEST (spiHoldsSsAsAHighOutputBeforeTheUnitStarts)
{
    bool ssDriven = false;

    // An SS that was an input held low would throw the unit out of master
    // mode as soon as it started.
    arduino::pin (SS).input = LOW;
    arduino::spi.onEnable   = [&]
    {
        ssDriven = arduino::pin (SS).mode == OUTPUT && arduino::pin (SS).output == HIGH;
    };

    CHECK (adk::spi::begin (49));
    CHECK (ssDriven);
    CHECK (SPCR & _BV (MSTR));
}

TEST (spiTransferSwapsAByteWithTheSelectedChip)
{
    Echo chip {49};

    adk::spi::begin (49);
    select (49, true);

    CHECK (adk::spi::transfer (0x12) == 0x00);
    CHECK (adk::spi::transfer (0x34) == 0x12);
    CHECK (adk::spi::transfer (0x56) == 0x34);
    CHECK (chip.heard == "\x12\x34\x56");
    CHECK (fake::spiLog.strays == 0);
}

TEST (spiTransferWaitsForTheUnitToFinish)
{
    Echo chip {49};

    arduino::spi.polls = 40;
    adk::spi::begin (49);
    select (49, true);

    adk::spi::transfer (0xA5);
    CHECK (adk::spi::transfer (0x00) == 0xA5);
    CHECK ((SPSR & _BV (WCOL)) == 0);
}

TEST (spiTransferWithNoChipSelectedReadsAnIdleLine)
{
    Echo chip {49};

    adk::spi::begin (49);

    CHECK (adk::spi::transfer (0x12) == 0xFF);
    CHECK (chip.heard.empty ());
    CHECK (fake::spiLog.strays == 1);
}

TEST (spiLetsOneChipSelectWithSs)
{
    Echo chip {SS};

    CHECK (adk::spi::begin (SS));
    CHECK (adk::spi::begin (48));

    select (SS, true);
    adk::spi::transfer (0x42);
    CHECK (chip.heard == "\x42");
}

TEST (spiRefusesASecondChipSelectingWithSs)
{
    CHECK (adk::spi::begin (SS));
    CHECK (!adk::spi::begin (SS));
    CHECK (adk::fault () == adk::Fault::PinInUse);
    CHECK (adk::faultPin () == SS);
}

TEST (spiLetsSsSelectAgainAfterANewSetup)
{
    CHECK (adk::spi::begin (SS));

    adk::releaseClaims ();
    CHECK (adk::spi::begin (SS));
}

TEST (twoRfidReadersSelectingWithSsHaltSetup)
{
    adk::Rfid front {SS, 8};
    adk::Rfid back  {SS, 7};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == SS);
}

TEST (anSpiSelectPinUsedTwiceHaltsSetup)
{
    adk::Rfid front {49, 8};
    adk::Led  lamp  {49};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 49);
}
