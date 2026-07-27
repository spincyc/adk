#include "traffic_junction.h"

namespace adk {

    static constexpr uint32_t maximumDuration = 0x7fffffffu;

    TrafficInput::TrafficInput (bool pedestrianRequest,
                                bool healthy) noexcept
        : pedestrianRequestEvent (pedestrianRequest)
        , circuitHealthy         (healthy)
    {
    }

    TrafficJunction::TrafficJunction (const TrafficConfig& config) noexcept
        : config_                    (config)
        , phase_                     (TrafficPhase::AllRed)
        , nextAfterAllRed_           (TrafficPhase::MainGreen)
        , status_                    (Status::NotInitialized)
        , phaseSince_                (TimePoint (0))
        , lastUpdate_                (TimePoint (0))
        , pedestrianRequestPending_  (false)
        , initialized_               (false)
        , hasLastUpdate_             (false)
        , phaseChanged_              (false)
        , requestAccepted_           (false)
        , transitionCount_           (0)
    {
    }

    TrafficJunction::~TrafficJunction () noexcept
    {
        shutdown ();
    }

    Status TrafficJunction::initialize () noexcept
    {
        if (initialized_)
        {
            return Status::Ok;
        }

        if (!configValid ())
        {
            status_ = Status::InvalidArgument;
            phase_  = TrafficPhase::Fault;
            return status_;
        }

        resetState ();
        initialized_ = true;

        return Status::Ok;
    }

    Status TrafficJunction::reset () noexcept
    {
        if (!initialized_)
        {
            return Status::NotInitialized;
        }

        resetState ();
        return Status::Ok;
    }

    void TrafficJunction::shutdown () noexcept
    {
        phase_                    = TrafficPhase::AllRed;
        nextAfterAllRed_          = TrafficPhase::MainGreen;
        status_                   = Status::NotInitialized;
        phaseSince_               = TimePoint (0);
        lastUpdate_               = TimePoint (0);
        pedestrianRequestPending_ = false;
        initialized_              = false;
        hasLastUpdate_            = false;
        phaseChanged_             = false;
        requestAccepted_          = false;
        transitionCount_          = 0;
    }

    bool TrafficJunction::initialized () const noexcept
    {
        return initialized_;
    }

    Status TrafficJunction::update (TimePoint now,
                                    const TrafficInput& input) noexcept
    {
        if (!initialized_)
        {
            return Status::NotInitialized;
        }

        phaseChanged_   = false;
        requestAccepted_ = false;

        if (phase_ == TrafficPhase::Fault)
        {
            return status_;
        }

        if (!input.circuitHealthy)
        {
            enterFault (Status::HardwareFailure, now);
            return status_;
        }

        if (!timeValid (now))
        {
            enterFault (Status::InvalidArgument, now);
            return status_;
        }

        if (!hasLastUpdate_)
        {
            phaseSince_    = now;
            lastUpdate_    = now;
            hasLastUpdate_ = true;
        } else
        {
            lastUpdate_ = now;
        }

        if (input.pedestrianRequestEvent && !pedestrianRequestPending_)
        {
            pedestrianRequestPending_ = true;
            requestAccepted_          = true;
        }

        status_ = Status::Ok;

        if (deadlineDue (now))
        {
            switch (phase_)
            {
                case TrafficPhase::AllRed:
                    transitionTo (phaseAfterAllRed (), now);
                    break;

                case TrafficPhase::MainGreen:
                    transitionTo (TrafficPhase::MainYellow, now);
                    break;

                case TrafficPhase::MainYellow:
                    enterAllRedBefore (TrafficPhase::SideGreen, now);
                    break;

                case TrafficPhase::SideGreen:
                    transitionTo (TrafficPhase::SideYellow, now);
                    break;

                case TrafficPhase::SideYellow:
                    enterAllRedBefore (TrafficPhase::MainGreen, now);
                    break;

                case TrafficPhase::PedestrianWalk:
                    transitionTo (TrafficPhase::PedestrianClearance, now);
                    break;

                case TrafficPhase::PedestrianClearance:
                    enterAllRedBefore (TrafficPhase::MainGreen, now);
                    break;

                case TrafficPhase::Fault:
                    break;
            }
        }

        if (!signalsValid (signalsForPhase ()))
        {
            enterFault (Status::HardwareFailure, now);
        }

        return status_;
    }

    TrafficSnapshot TrafficJunction::snapshot () const noexcept
    {
        const Duration duration = phaseDuration ();
        const bool hasDeadline = initialized_ &&
                                 hasLastUpdate_ &&
                                 phase_ != TrafficPhase::Fault;
        const TimePoint deadline (
            phaseSince_.milliseconds () + duration.milliseconds ());
        const TrafficSnapshot result = {
            phase_,
            status_,
            signalsForPhase (),
            phaseSince_,
            deadline,
            pedestrianRequestPending_,
            phaseChanged_,
            requestAccepted_,
            hasDeadline,
            transitionCount_};

        return result;
    }

    void TrafficJunction::resetState () noexcept
    {
        phase_                    = TrafficPhase::AllRed;
        nextAfterAllRed_          = TrafficPhase::MainGreen;
        status_                   = Status::Ok;
        phaseSince_               = TimePoint (0);
        lastUpdate_               = TimePoint (0);
        pedestrianRequestPending_ = false;
        hasLastUpdate_            = false;
        phaseChanged_             = false;
        requestAccepted_          = false;
        transitionCount_          = 0;
    }

    bool TrafficJunction::configValid () const noexcept
    {
        const Duration durations[] = {
            config_.startupAllRed,
            config_.vehicleAllRed,
            config_.mainGreen,
            config_.mainYellow,
            config_.sideGreen,
            config_.sideYellow,
            config_.pedestrianWalk,
            config_.pedestrianClearance};

        for (const Duration duration : durations)
        {
            if (duration.milliseconds () == 0 ||
                duration.milliseconds () > maximumDuration)
            {
                return false;
            }
        }

        return true;
    }

    bool TrafficJunction::timeValid (TimePoint now) const noexcept
    {
        return !hasLastUpdate_ ||
               now.elapsedSince (lastUpdate_).milliseconds () <= maximumDuration;
    }

    bool TrafficJunction::deadlineDue (TimePoint now) const noexcept
    {
        return now.elapsedSince (phaseSince_) >= phaseDuration ();
    }

    bool TrafficJunction::signalsValid (const TrafficSignals& signals) const noexcept
    {
        const unsigned int mainActive = static_cast<unsigned int> (signals.mainRed) +
                                        static_cast<unsigned int> (signals.mainYellow) +
                                        static_cast<unsigned int> (signals.mainGreen);
        const unsigned int sideActive = static_cast<unsigned int> (signals.sideRed) +
                                        static_cast<unsigned int> (signals.sideYellow) +
                                        static_cast<unsigned int> (signals.sideGreen);

        return mainActive == 1 &&
               sideActive == 1 &&
               signals.pedestrianStop != signals.pedestrianWalk &&
               !(signals.mainGreen && signals.sideGreen) &&
               !(signals.mainGreen && signals.pedestrianWalk) &&
               !(signals.sideGreen && signals.pedestrianWalk);
    }

    Duration TrafficJunction::phaseDuration () const noexcept
    {
        switch (phase_)
        {
            case TrafficPhase::AllRed:
                return transitionCount_ == 0 ? config_.startupAllRed
                                             : config_.vehicleAllRed;
            case TrafficPhase::MainGreen:           return config_.mainGreen;
            case TrafficPhase::MainYellow:          return config_.mainYellow;
            case TrafficPhase::SideGreen:           return config_.sideGreen;
            case TrafficPhase::SideYellow:          return config_.sideYellow;
            case TrafficPhase::PedestrianWalk:      return config_.pedestrianWalk;
            case TrafficPhase::PedestrianClearance: return config_.pedestrianClearance;
            case TrafficPhase::Fault:               return Duration (0);
        }

        return Duration (0);
    }

    TrafficSignals TrafficJunction::signalsForPhase () const noexcept
    {
        TrafficSignals signals = {
            true,
            false,
            false,
            true,
            false,
            false,
            true,
            false};

        switch (phase_)
        {
            case TrafficPhase::MainGreen:
                signals.mainRed   = false;
                signals.mainGreen = true;
                break;

            case TrafficPhase::MainYellow:
                signals.mainRed    = false;
                signals.mainYellow = true;
                break;

            case TrafficPhase::SideGreen:
                signals.sideRed   = false;
                signals.sideGreen = true;
                break;

            case TrafficPhase::SideYellow:
                signals.sideRed    = false;
                signals.sideYellow = true;
                break;

            case TrafficPhase::PedestrianWalk:
                signals.pedestrianStop = false;
                signals.pedestrianWalk = true;
                break;

            case TrafficPhase::AllRed:
            case TrafficPhase::PedestrianClearance:
            case TrafficPhase::Fault:
                break;
        }

        return signals;
    }

    TrafficPhase TrafficJunction::phaseAfterAllRed () const noexcept
    {
        if (nextAfterAllRed_ == TrafficPhase::MainGreen &&
            pedestrianRequestPending_)
        {
            return TrafficPhase::PedestrianWalk;
        }

        return nextAfterAllRed_;
    }

    void TrafficJunction::transitionTo (TrafficPhase phase, TimePoint now) noexcept
    {
        phase_        = phase;
        phaseSince_   = now;
        phaseChanged_ = true;
        ++transitionCount_;

        if (phase == TrafficPhase::PedestrianWalk)
        {
            pedestrianRequestPending_ = false;
        }
    }

    void TrafficJunction::enterAllRedBefore (TrafficPhase phase,
                                             TimePoint    now) noexcept
    {
        nextAfterAllRed_ = phase;
        transitionTo (TrafficPhase::AllRed, now);
    }

    void TrafficJunction::enterFault (Status status, TimePoint now) noexcept
    {
        status_       = status;
        phase_        = TrafficPhase::Fault;
        phaseSince_   = now;
        phaseChanged_ = true;
        ++transitionCount_;
    }
}
