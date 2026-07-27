#include "status.h"

namespace adk {

    const char* statusName (Status status) noexcept
    {
        switch (status)
        {
        case Status::Ok:               return "ok";
        case Status::InvalidArgument:  return "invalid argument";
        case Status::InvalidPin:       return "invalid pin";
        case Status::Unsupported:      return "unsupported";
        case Status::ResourceBusy:     return "resource busy";
        case Status::NotInitialized:   return "not initialized";
        case Status::CapacityExceeded: return "capacity exceeded";
        case Status::HardwareFailure:  return "hardware failure";
        }

        return "unknown";
    }
}
