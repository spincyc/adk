#pragma once

#include "inert_load_interlock.h"
#include "pump_output.h"

namespace adk {

    struct SimulatedLoadSnapshot
    {
        SimulatedLoad active;
        Status        lastStatus;
    };

    struct InertLoadPanel
    {
        InertLoadPanel (PumpOutput& fanIndicator, PumpOutput& pumpIndicator,
                        PumpOutput& heaterIndicator) noexcept;
        ~InertLoadPanel () noexcept;

        InertLoadPanel (const InertLoadPanel&)            = delete;
        InertLoadPanel& operator= (const InertLoadPanel&) = delete;
        InertLoadPanel (InertLoadPanel&&)                 = delete;
        InertLoadPanel& operator= (InertLoadPanel&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status select     (SimulatedLoad load) noexcept;

        SimulatedLoadSnapshot snapshot    () const noexcept;
        bool                  initialized () const noexcept;

      private:
        PumpOutput* outputFor     (SimulatedLoad load) noexcept;
        bool        validLoad     (SimulatedLoad load) const noexcept;
        void        bestEffortOff () noexcept;

        PumpOutput*   fan_;
        PumpOutput*   pump_;
        PumpOutput*   heater_;
        SimulatedLoad active_;
        Status        status_;
        bool          initialized_;
    };

    struct PanelPumpOutput final : PumpOutput
    {
        explicit PanelPumpOutput (InertLoadPanel& panel) noexcept;
        ~PanelPumpOutput         () noexcept override;

        PanelPumpOutput            (const PanelPumpOutput&) = delete;
        PanelPumpOutput& operator= (const PanelPumpOutput&) = delete;
        PanelPumpOutput            (PanelPumpOutput&&)      = delete;
        PanelPumpOutput& operator= (PanelPumpOutput&&)      = delete;

        Status    initialize  () noexcept override;
        void      shutdown    () noexcept override;
        Status    setState    (PumpState state) noexcept override;
        PumpState state       () const noexcept override;
        bool      initialized () const noexcept override;

      private:
        InertLoadPanel* panel_;
        bool            initialized_;
    };
} // namespace adk
