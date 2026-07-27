#include "status.h"

namespace adk {

    const char* statusName (Status status) noexcept
    {
        switch (status.error ())
        {
        case StatusCode::Ok:               return "ok";
        case StatusCode::InvalidArgument:  return "invalid argument";
        case StatusCode::InvalidPin:       return "invalid pin";
        case StatusCode::Unsupported:      return "unsupported";
        case StatusCode::ResourceBusy:     return "resource busy";
        case StatusCode::NotInitialized:   return "not initialized";
        case StatusCode::CapacityExceeded: return "capacity exceeded";
        case StatusCode::HardwareFailure:  return "hardware failure";
        }

        return "unknown";
    }
}
