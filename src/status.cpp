#include "status.h"

namespace adk {

    const Status Status::Ok               (StatusCode::Ok);
    const Status Status::InvalidArgument  (StatusCode::InvalidArgument);
    const Status Status::InvalidPin       (StatusCode::InvalidPin);
    const Status Status::Unsupported      (StatusCode::Unsupported);
    const Status Status::ResourceBusy     (StatusCode::ResourceBusy);
    const Status Status::NotInitialized   (StatusCode::NotInitialized);
    const Status Status::CapacityExceeded (StatusCode::CapacityExceeded);
    const Status Status::HardwareFailure  (StatusCode::HardwareFailure);

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
