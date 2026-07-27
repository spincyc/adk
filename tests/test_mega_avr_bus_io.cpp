#include "mega_avr_bus_io.h"

#include <cstdlib>
#include <type_traits>

namespace {

    void require (bool condition)
    {
        if (!condition)
        {
            std::abort ();
        }
    }

    struct TestRegisters final : adk::MegaAvrBusRegisters
    {
        static constexpr uint8_t registerCount = 14;

        uint8_t  values[registerCount] = {};
        uint8_t  twiStates[16]         = {};
        uint8_t  receiveBytes[8]       = {};
        uint8_t  twiStateCount         = 0;
        uint8_t  twiStateIndex         = 0;
        uint8_t  receiveCount          = 0;
        uint8_t  receiveIndex          = 0;
        uint32_t now                   = 0;
        bool     stallTwi              = false;
        bool     stallSpi              = false;
        bool     twiPending             = false;
        uint8_t  stallTwiAfterStates     = UINT8_MAX;

        uint8_t read (adk::MegaAvrRegister reg) noexcept override
        {
            const uint8_t index = static_cast<uint8_t> (reg);

            if (reg == adk::MegaAvrRegister::Twcr && (values[index] & 0x80U) == 0 &&
                twiPending && !stallTwi && twiStateIndex < twiStateCount &&
                twiStateIndex < stallTwiAfterStates)
            {
                values[index] |= 0x80U;
                twiPending = false;
                values[static_cast<uint8_t> (adk::MegaAvrRegister::Twsr)] =
                    twiStates[twiStateIndex++];

                if ((twiStates[twiStateIndex - 1U] == 0x50U ||
                     twiStates[twiStateIndex - 1U] == 0x58U) &&
                    receiveIndex < receiveCount)
                {
                    values[static_cast<uint8_t> (adk::MegaAvrRegister::Twdr)] =
                        receiveBytes[receiveIndex++];
                }
            }

            return values[index];
        }

        void write (adk::MegaAvrRegister reg, uint8_t value) noexcept override
        {
            const uint8_t index = static_cast<uint8_t> (reg);

            if (reg == adk::MegaAvrRegister::Twcr && (value & 0x80U) != 0 &&
                (value & 0x10U) == 0)
            {
                values[index] = static_cast<uint8_t> (value & ~0x80U);
                twiPending   = true;
                return;
            }

            values[index] = value;

            if (reg == adk::MegaAvrRegister::Twcr && (value & 0x10U) != 0 &&
                !stallTwi && twiStateIndex < stallTwiAfterStates)
            {
                values[index] &= static_cast<uint8_t> (~0x10U);
            }

            if (reg == adk::MegaAvrRegister::Spdr && !stallSpi)
            {
                values[static_cast<uint8_t> (adk::MegaAvrRegister::Spsr)] |= 0x80U;
                values[index] = static_cast<uint8_t> (value ^ 0xffU);
            }
        }

        uint32_t microsecondsNow () noexcept override
        {
            now += 250;
            return now;
        }

        void scriptTwi (const uint8_t* states, uint8_t size) noexcept
        {
            twiStateCount = size;
            twiStateIndex = 0;

            for (uint8_t index = 0; index < size; ++index)
            {
                twiStates[index] = states[index];
            }
        }
    };

    uint8_t& reg (TestRegisters& registers, adk::MegaAvrRegister selected) noexcept
    {
        return registers.values[static_cast<uint8_t> (selected)];
    }

    void provesI2cConfigurationAndRestoration ()
    {
        TestRegisters registers;
        reg (registers, adk::MegaAvrRegister::Twbr)  = 7;
        reg (registers, adk::MegaAvrRegister::Twsr)  = 3;
        reg (registers, adk::MegaAvrRegister::Twar)  = 9;
        reg (registers, adk::MegaAvrRegister::Twcr)  = 2;
        reg (registers, adk::MegaAvrRegister::Ddrd)  = 0xa5;
        reg (registers, adk::MegaAvrRegister::Portd) = 0x5a;

        adk::MegaAvrI2cIo io (registers);

        require (io.configure ().ok ());
        require (reg (registers, adk::MegaAvrRegister::Twbr) == 72);
        require ((reg (registers, adk::MegaAvrRegister::Twsr) & 3U) == 0);
        require ((reg (registers, adk::MegaAvrRegister::Ddrd) & 3U) == 0);
        require ((reg (registers, adk::MegaAvrRegister::Portd) & 3U) == 0);

        io.disable ();

        require (reg (registers, adk::MegaAvrRegister::Twbr) == 7);
        require (reg (registers, adk::MegaAvrRegister::Twsr) == 3);
        require (reg (registers, adk::MegaAvrRegister::Twar) == 9);
        require (reg (registers, adk::MegaAvrRegister::Twcr) == 2);
        require (reg (registers, adk::MegaAvrRegister::Ddrd) == 0xa5);
        require (reg (registers, adk::MegaAvrRegister::Portd) == 0x5a);
    }

    void provesI2cCombinedTransferAndTimeout ()
    {
        TestRegisters     registers;
        adk::MegaAvrI2cIo io (registers);
        const uint8_t     states[] = {0x08, 0x18, 0x28, 0x10, 0x40, 0x50, 0x58};
        const uint8_t     write[]  = {0x22};
        uint8_t           read[2]  = {};

        registers.receiveBytes[0] = 0x41;
        registers.receiveBytes[1] = 0x42;
        registers.receiveCount    = 2;
        registers.scriptTwi (states, sizeof (states));

        require (io.configure ().ok ());
        require (io.transfer (0x50, write, sizeof (write), read, sizeof (read),
                              adk::Duration (10))
                     .ok ());
        require (read[0] == 0x41 && read[1] == 0x42);
        require (io.transfer (0x50, write, sizeof (write), nullptr, 0,
                              adk::Duration (UINT32_MAX))
                     .error () == adk::StatusCode::InvalidArgument);

        registers.stallTwi = true;
        require (
            io.transfer (0x50, write, sizeof (write), nullptr, 0, adk::Duration (1))
                .error () == adk::StatusCode::HardwareFailure);
        require (reg (registers, adk::MegaAvrRegister::Twcr) == 0);
    }

    void provesI2cProtocolFailureStops ()
    {
        TestRegisters     registers;
        adk::MegaAvrI2cIo io (registers);
        const uint8_t     states[] = {0x08, 0x20};
        const uint8_t     value    = 0x33;

        registers.scriptTwi (states, sizeof (states));

        require (io.configure ().ok ());
        require (
            io.transfer (0x50, &value, 1, nullptr, 0, adk::Duration (5)).error () ==
            adk::StatusCode::HardwareFailure);
        require ((reg (registers, adk::MegaAvrRegister::Twcr) & 0x10U) == 0);
    }

    void provesI2cLaterPhaseTimeoutsAndRollover ()
    {
        const uint8_t states[] = {0x08, 0x18, 0x28, 0x10, 0x40, 0x50, 0x58};
        const uint8_t value    = 0x33;

        for (uint8_t completedStates = 1; completedStates < 7; ++completedStates)
        {
            TestRegisters     registers;
            adk::MegaAvrI2cIo io (registers);
            uint8_t           read[2] = {};

            registers.scriptTwi          (states, sizeof (states));
            registers.stallTwiAfterStates = completedStates;
            require (io.configure ().ok ());
            require (io.transfer (0x50, &value, 1, read, 2, adk::Duration (2))
                         .error () == adk::StatusCode::HardwareFailure);
            require (reg (registers, adk::MegaAvrRegister::Twcr) == 0);
        }

        TestRegisters     rolloverRegisters;
        adk::MegaAvrI2cIo rolloverIo (rolloverRegisters);
        const uint8_t     rolloverStates[] = {0x08, 0x18, 0x28};

        rolloverRegisters.now = UINT32_MAX - 300U;
        rolloverRegisters.scriptTwi (rolloverStates, sizeof (rolloverStates));
        require                     (rolloverIo.configure ().ok ());
        require                     (rolloverIo
                                         .transfer (0x50, &value, 1, nullptr, 0,
                                                    adk::Duration (2))
                                         .ok ());
    }

    void provesSpiFixedPinsAndRestoration ()
    {
        TestRegisters registers;
        reg (registers, adk::MegaAvrRegister::Spcr)  = 0x11;
        reg (registers, adk::MegaAvrRegister::Spsr)  = 0x22;
        reg (registers, adk::MegaAvrRegister::Ddrb)  = 0x88;
        reg (registers, adk::MegaAvrRegister::Portb) = 0x44;
        reg (registers, adk::MegaAvrRegister::Ddrl)  = 0xa0;
        reg (registers, adk::MegaAvrRegister::Portl) = 0x50;

        adk::MegaAvrSpiIo io (registers);

        require (io.configureMaster ().ok ());
        require ((reg (registers, adk::MegaAvrRegister::Ddrb) & 0x07U) == 0x07U);
        require ((reg (registers, adk::MegaAvrRegister::Ddrb) & 0x08U) == 0);
        require (io.configureChipSelect (48).error () == adk::StatusCode::Unsupported);
        require (io.configureChipSelect (49).ok ());
        require ((reg (registers, adk::MegaAvrRegister::Ddrl) & 1U) != 0);
        require ((reg (registers, adk::MegaAvrRegister::Portl) & 1U) != 0);

        io.disable ();

        require ((reg (registers, adk::MegaAvrRegister::Portl) & 1U) != 0);
        require (reg (registers, adk::MegaAvrRegister::Ddrb) == 0x88);
        require (reg (registers, adk::MegaAvrRegister::Portb) == 0x44);

        io.releaseChipSelect (49);

        require (reg (registers, adk::MegaAvrRegister::Ddrl) == 0xa0);
        require (reg (registers, adk::MegaAvrRegister::Portl) == 0x50);
    }

    void provesSpiModesBytesAndTimeout ()
    {
        TestRegisters          registers;
        adk::MegaAvrSpiIo      io (registers);
        const adk::SpiSettings settings = {2000000, 3, false};
        const uint8_t          write[]  = {0x00, 0xa5};
        uint8_t                read[2]  = {};

        require (io.configureMaster ().ok ());
        require (io.configureChipSelect (49).ok ());
        require (io.beginTransaction (settings, 49).ok ());
        require ((reg (registers, adk::MegaAvrRegister::Spcr) & 0x3cU) == 0x3cU);
        require ((reg (registers, adk::MegaAvrRegister::Portl) & 1U) == 0);
        require (io.transfer (write, read, 2, adk::Duration (2)).ok ());
        require (read[0] == 0xff && read[1] == 0x5a);
        require (io.endTransaction (49).ok ());
        require ((reg (registers, adk::MegaAvrRegister::Portl) & 1U) != 0);

        require (io.beginTransaction ({3000000, 0, true}, 49).error () ==
                 adk::StatusCode::Unsupported);
        require (io.endTransaction (49).ok ());
        require (io.beginTransaction ({1000000, 0, true}, 49).ok ());
        require (io.transfer (write, read, 1, adk::Duration (UINT32_MAX))
                     .error () == adk::StatusCode::InvalidArgument);
        registers.stallSpi = true;

        reg (registers, adk::MegaAvrRegister::Spsr) &= 0x7fU;

        require (io.transfer (write, read, 1, adk::Duration (1)).error () ==
                 adk::StatusCode::HardwareFailure);
        require (io.endTransaction (49).ok ());
    }

    void provesEverySpiSettingEncoding ()
    {
        struct ClockCase
        {
            uint32_t clockHz;
            uint8_t  divider;
            uint8_t  doubleSpeed;
        };

        const ClockCase clocks[] = {
            {8000000UL, 0, 1}, {4000000UL, 0, 0}, {2000000UL, 1, 1},
            {1000000UL, 1, 0}, {500000UL, 2, 1},  {250000UL, 2, 0},
            {125000UL, 3, 0}
        };

        for (const ClockCase& clock : clocks)
        {
            for (uint8_t mode = 0; mode < 4; ++mode)
            {
                for (uint8_t bitOrder = 0; bitOrder < 2; ++bitOrder)
                {
                    TestRegisters     registers;
                    adk::MegaAvrSpiIo io (registers);
                    const bool        mostSignificantBitFirst = bitOrder == 0;

                    require (io.configureMaster ().ok ());
                    require (io.configureChipSelect (49).ok ());
                    require (io.beginTransaction (
                                   {clock.clockHz, mode, mostSignificantBitFirst}, 49)
                                 .ok ());

                    const uint8_t expectedControl = static_cast<uint8_t> (
                        0x50U | clock.divider |
                        (mostSignificantBitFirst ? 0U : 0x20U) |
                        ((mode & 2U) != 0 ? 0x08U : 0U) |
                        ((mode & 1U) != 0 ? 0x04U : 0U));
                    require (reg (registers, adk::MegaAvrRegister::Spcr) ==
                             expectedControl);
                    require ((reg (registers, adk::MegaAvrRegister::Spsr) & 1U) ==
                             clock.doubleSpeed);
                    require (io.endTransaction (49).ok ());
                }
            }
        }

        TestRegisters     rolloverRegisters;
        adk::MegaAvrSpiIo rolloverIo (rolloverRegisters);
        const uint8_t     value = 0x55;

        rolloverRegisters.now = UINT32_MAX - 300U;
        require (rolloverIo.configureMaster ().ok ());
        require (rolloverIo.configureChipSelect (49).ok ());
        require (rolloverIo.beginTransaction ({1000000UL, 0, true}, 49).ok ());
        require (rolloverIo.transfer (&value, nullptr, 1, adk::Duration (1)).ok ());
        require (rolloverIo.endTransaction (49).ok ());
    }

    void provesTraits ()
    {
        static_assert (!std::is_copy_constructible<adk::MegaAvrI2cIo>::value,
                       "I2C backend retains a register relationship");
        static_assert (!std::is_copy_constructible<adk::MegaAvrSpiIo>::value,
                       "SPI backend retains snapshots");
    }

#if !defined(__AVR_ATmega2560__)
    void provesDirectBackendsRejectOtherTargets ()
    {
        adk::MegaAvrI2cIo i2c;
        adk::MegaAvrSpiIo spi;

        require (i2c.configure ().error () == adk::StatusCode::Unsupported);
        require (spi.configureMaster ().error () == adk::StatusCode::Unsupported);
    }
#endif
} // namespace

int main ()
{
    provesI2cConfigurationAndRestoration ();

    provesI2cCombinedTransferAndTimeout ();

    provesI2cProtocolFailureStops          ();
    provesI2cLaterPhaseTimeoutsAndRollover ();

    provesSpiFixedPinsAndRestoration ();

    provesSpiModesBytesAndTimeout ();
    provesEverySpiSettingEncoding ();

    provesTraits ();

#if !defined(__AVR_ATmega2560__)
    provesDirectBackendsRejectOtherTargets ();
#endif
}
