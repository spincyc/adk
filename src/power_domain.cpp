#include "power_domain.h"

namespace adk {

    PowerDomain::~PowerDomain () noexcept = default;

    ExternalPowerDomainGate::ExternalPowerDomainGate () noexcept
        : admitted_ (false)
    {
    }

    void ExternalPowerDomainGate::admit () noexcept
    {
        admitted_ = true;
    }

    void ExternalPowerDomainGate::revoke () noexcept
    {
        admitted_ = false;
    }

    bool ExternalPowerDomainGate::commandAdmitted () const noexcept
    {
        return admitted_;
    }
}
