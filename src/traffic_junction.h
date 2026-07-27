#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct TrafficPhase : uint8_t
    {
        AllRed,
        MainGreen,
        MainYellow,
        SideGreen,
        SideYellow,
        PedestrianWalk,
        PedestrianClearance,
        Fault
    };

    struct TrafficInput
    {
        TrafficInput (bool pedestrianRequestEvent = false,
                      bool circuitHealthy         = true) noexcept;

        bool pedestrianRequestEvent;
        bool circuitHealthy;
    };

    struct TrafficConfig
    {
        Duration startupAllRed       = Duration (1000);
        Duration vehicleAllRed       = Duration (500);
        Duration mainGreen           = Duration (5000);
        Duration mainYellow          = Duration (1500);
        Duration sideGreen           = Duration (4000);
        Duration sideYellow          = Duration (1500);
        Duration pedestrianWalk      = Duration (5000);
        Duration pedestrianClearance = Duration (2000);
    };

    struct TrafficSignals
    {
        bool mainRed;
        bool mainYellow;
        bool mainGreen;
        bool sideRed;
        bool sideYellow;
        bool sideGreen;
        bool pedestrianStop;
        bool pedestrianWalk;
    };

    struct TrafficSnapshot
    {
        TrafficPhase   phase;
        Status         status;
        TrafficSignals signals;
        TimePoint      phaseSince;
        TimePoint      nextDeadline;
        bool           pedestrianRequestPending;
        bool           phaseChanged;
        bool           requestAccepted;
        bool           hasDeadline;
        uint32_t       transitionCount;
    };

    struct TrafficJunction
    {
        explicit TrafficJunction (const TrafficConfig& config) noexcept;
        ~TrafficJunction         () noexcept;

        Status initialize () noexcept;
        Status reset      () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now, const TrafficInput& input) noexcept;

        bool            initialized () const noexcept;
        TrafficSnapshot snapshot    () const noexcept;

      private:
        void resetState () noexcept;

        bool           configValid       () const noexcept;
        bool           timeValid         (TimePoint now) const noexcept;
        bool           deadlineDue       (TimePoint now) const noexcept;
        bool           signalsValid      (const TrafficSignals& signals) const noexcept;
        Duration       phaseDuration     () const noexcept;
        TrafficSignals signalsForPhase   () const noexcept;
        TrafficPhase   phaseAfterAllRed  () const noexcept;
        void           transitionTo      (TrafficPhase phase, TimePoint now) noexcept;
        void           enterAllRedBefore (TrafficPhase phase, TimePoint now) noexcept;
        void           enterFault        (Status status, TimePoint now) noexcept;

        TrafficConfig config_;
        TrafficPhase  phase_;
        TrafficPhase  nextAfterAllRed_;
        Status        status_;
        TimePoint     phaseSince_;
        TimePoint     lastUpdate_;
        bool          pedestrianRequestPending_;
        bool          initialized_;
        bool          hasLastUpdate_;
        bool          phaseChanged_;
        bool          requestAccepted_;
        uint32_t      transitionCount_;
    };
}
