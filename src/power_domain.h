#pragma once

namespace adk {

    struct PowerDomain
    {
        virtual ~PowerDomain () noexcept;

        virtual bool commandAdmitted () const noexcept = 0;
    };

    struct ExternalPowerDomainGate final : PowerDomain
    {
        ExternalPowerDomainGate () noexcept;

        void admit  () noexcept;
        void revoke () noexcept;

        bool commandAdmitted () const noexcept override;

      private:
        bool admitted_;
    };
}
