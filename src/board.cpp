#include "board.h"

namespace adk {

    namespace {
        bool pwmPin (PinId pin) noexcept
        {
            return (pin >= 2 && pin <= 13)
                || (pin >= 44 && pin <= 46);
        }

        bool analogInputPin (PinId pin) noexcept
        {
            return pin >= 54 && pin <= 69;
        }

        bool externalInterruptPin (PinId pin) noexcept
        {
            return pin == 2
                || pin == 3
                || (pin >= 18 && pin <= 21);
        }
    }

    bool Mega2560Board::validPin (PinId pin) noexcept
    {
        return pin <= 69;
    }

    bool Mega2560Board::supports (
        PinId         pin,
        PinCapability capability) noexcept
    {
        if (!validPin (pin))
        {
            return false;
        }

        switch (capability)
        {
        case PinCapability::DigitalInput:
        case PinCapability::DigitalOutput:
            return true;
        case PinCapability::PwmOutput:
            return pwmPin (pin);
        case PinCapability::AnalogInput:
            return analogInputPin (pin);
        case PinCapability::ExternalInterrupt:
            return externalInterruptPin (pin);
        }

        return false;
    }

    Status Mega2560Board::pwmTimer (PinId pin, uint8_t& timer) noexcept
    {
        switch (pin)
        {
        case 4:
        case 13:
            timer = 0;
            return Status::Ok;
        case 11:
        case 12:
            timer = 1;
            return Status::Ok;
        case 9:
        case 10:
            timer = 2;
            return Status::Ok;
        case 2:
        case 3:
        case 5:
            timer = 3;
            return Status::Ok;
        case 6:
        case 7:
        case 8:
            timer = 4;
            return Status::Ok;
        case 44:
        case 45:
        case 46:
            timer = 5;
            return Status::Ok;
        default:
            return validPin (pin) ? Status::Unsupported : Status::InvalidPin;
        }
    }
}
