#include "fake_spi.h"

#include <Arduino.h>

namespace fake {

    SpiLog spiLog = {0, 0, 0, 0};

    namespace {

        SpiChip* chips = nullptr;

        bool selected (const SpiChip* chip)
        {
            const arduino::PinState& pin = arduino::pin (chip->select_);
            return pin.mode == OUTPUT && pin.output == LOW;
        }

        void watch (uint8_t pin, uint8_t value)
        {
            for (SpiChip* chip = chips; chip; chip = chip->next_)
            {
                if (chip->select_ == pin && value == HIGH)
                {
                    chip->deselected ();
                }
            }
        }

        uint8_t transfer (uint8_t byte)
        {
            SpiChip* target = nullptr;

            ++spiLog.bytes;

            for (SpiChip* chip = chips; chip; chip = chip->next_)
            {
                if (selected (chip))
                {
                    spiLog.clashes += target ? 1 : 0;
                    target = chip;
                }
            }

            if (!target)
            {
                // Nothing drives MISO, and an idle line reads high.
                ++spiLog.strays;
                return 0xFF;
            }

            return target->exchange (byte);
        }
    }

    SpiChip::SpiChip (adk::Pin select)
        : select_ (select)
        , next_   (chips)
    {
        // The first chip of a test starts a fresh log.
        if (!chips)
        {
            spiLog                  = {0, 0, 0, 0};
            arduino::onDigitalWrite = watch;
            arduino::spi.onTransfer = transfer;
            arduino::spi.onEnable   = [] { ++spiLog.begins; };
        }

        chips = this;
    }

    SpiChip::~SpiChip ()
    {
        for (SpiChip** link = &chips; *link; link = &(*link)->next_)
        {
            if (*link == this)
            {
                *link = next_;
                break;
            }
        }

        if (!chips)
        {
            arduino::onDigitalWrite = nullptr;
            arduino::spi.onTransfer = nullptr;
            arduino::spi.onEnable   = nullptr;
        }
    }

    void SpiChip::deselected ()
    {
    }
}
