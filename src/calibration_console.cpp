#include "calibration_console.h"

namespace adk {

    static constexpr uint32_t maximumDuration = 0x7fffffffu;

    CalibrationConsoleConfig::CalibrationConsoleConfig (
        uint16_t lower, uint16_t upper, uint16_t separation,
        Duration acknowledgementDuration) noexcept
        : lowerLimit (lower), upperLimit (upper), minimumSeparation (separation),
          acknowledgement (acknowledgementDuration)
    {
    }

    CalibrationConsoleInput::CalibrationConsoleInput () noexcept
        : joystickX (0), joystickY (0), selectEvent (false), encoderDelta (0),
          cancelEvent (false), inputValid (true)
    {
    }

    CalibrationConsole::CalibrationConsole (
        const CalibrationConsoleConfig& config) noexcept
        : config_       (config)
        , snapshot_     {CalibrationConsoleState::Fault,
                         CalibrationField::Minimum,
                         0,
                         0,
                         0,
                         0,
                         false,
                         StatusCode::NotInitialized}
        , stateSince_    (TimePoint (0))
        , lastUpdate_    (TimePoint (0))
        , initialized_   (false)
        , hasLastUpdate_ (false)
    {
    }

    Status CalibrationConsole::initialize (uint16_t initialMinimum,
                                           uint16_t initialMaximum) noexcept
    {
        if (initialized_ &&
            snapshot_.state != CalibrationConsoleState::Fault)
        {
            return StatusCode::Ok;
        }

        if (!configValid () || !initialRangeValid (initialMinimum, initialMaximum))
        {
            initialized_      = false;
            hasLastUpdate_    = false;
            snapshot_.state   = CalibrationConsoleState::Fault;
            snapshot_.status  = StatusCode::InvalidConfiguration;
            snapshot_.changed = false;
            return snapshot_.status;
        }

        snapshot_.state            = CalibrationConsoleState::Selecting;
        snapshot_.field            = CalibrationField::Minimum;
        snapshot_.committedMinimum = initialMinimum;
        snapshot_.committedMaximum = initialMaximum;
        snapshot_.previewMinimum   = initialMinimum;
        snapshot_.previewMaximum   = initialMaximum;
        snapshot_.changed          = false;
        snapshot_.status           = StatusCode::Ok;
        stateSince_                = TimePoint (0);
        lastUpdate_                = TimePoint (0);
        initialized_               = true;
        hasLastUpdate_             = false;

        return snapshot_.status;
    }

    void CalibrationConsole::shutdown () noexcept
    {
        snapshot_.state          = CalibrationConsoleState::Selecting;
        snapshot_.previewMinimum = snapshot_.committedMinimum;
        snapshot_.previewMaximum = snapshot_.committedMaximum;
        snapshot_.changed        = false;
        snapshot_.status         = StatusCode::NotInitialized;
        initialized_             = false;
        hasLastUpdate_           = false;
    }

    Status CalibrationConsole::update (TimePoint                      now,
                                       const CalibrationConsoleInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!timeValid (now))
        {
            enterFault (StatusCode::InvalidArgument, now);
            return snapshot_.status;
        }

        lastUpdate_    = now;
        hasLastUpdate_ = true;

        if (snapshot_.state == CalibrationConsoleState::Fault)
        {
            return snapshot_.status;
        }

        if (!input.inputValid)
        {
            enterFault (StatusCode::HardwareFailure, now);
            return snapshot_.status;
        }

        snapshot_.status = StatusCode::Ok;

        if (snapshot_.state == CalibrationConsoleState::Committed ||
            snapshot_.state == CalibrationConsoleState::Cancelled)
        {
            if (acknowledgementDue (now))
            {
                snapshot_.state = CalibrationConsoleState::Selecting;
                stateSince_     = now;
            }
            return snapshot_.status;
        }

        if (snapshot_.state == CalibrationConsoleState::Selecting)
        {
            if (input.joystickX < -selectionThreshold)
            {
                snapshot_.field = CalibrationField::Minimum;
            }
            else if (input.joystickX > selectionThreshold)
            {
                snapshot_.field = CalibrationField::Maximum;
            }

            if (input.selectEvent)
            {
                beginEditing (now);
            }

            return snapshot_.status;
        }

        if (input.cancelEvent)
        {
            cancel (now);
            return snapshot_.status;
        }

        if (input.joystickY < -selectionThreshold ||
            input.joystickY > selectionThreshold)
        {
            const uint16_t candidate = coarseCandidate (input.joystickY);

            if (snapshot_.field == CalibrationField::Minimum)
            {
                snapshot_.previewMinimum = candidate;
            }
            else
            {
                snapshot_.previewMaximum = candidate;
            }
        }

        if (input.encoderDelta != 0)
        {
            if (snapshot_.field == CalibrationField::Minimum)
            {
                snapshot_.previewMinimum =
                    clampCandidate (static_cast<int32_t> (snapshot_.previewMinimum) +
                                    input.encoderDelta);
            }
            else
            {
                snapshot_.previewMaximum =
                    clampCandidate (static_cast<int32_t> (snapshot_.previewMaximum) +
                                    input.encoderDelta);
            }
        }

        refreshChanged ();

        if (input.selectEvent)
        {
            commit (now);
        }

        return snapshot_.status;
    }

    bool CalibrationConsole::initialized () const noexcept
    {
        return initialized_;
    }

    CalibrationConsoleSnapshot CalibrationConsole::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool CalibrationConsole::configValid () const noexcept
    {
        const uint32_t acknowledgement = config_.acknowledgement.milliseconds ();
        const uint32_t range           = static_cast<uint32_t> (config_.upperLimit) -
                                         static_cast<uint32_t> (config_.lowerLimit);

        return config_.lowerLimit < config_.upperLimit &&
               config_.minimumSeparation <= range && acknowledgement > 0 &&
               acknowledgement <= maximumDuration;
    }

    bool CalibrationConsole::initialRangeValid (uint16_t initialMinimum,
                                                uint16_t initialMaximum) const noexcept
    {
        return initialMinimum >= config_.lowerLimit &&
               initialMaximum <= config_.upperLimit &&
               initialMinimum <= initialMaximum &&
               static_cast<uint32_t> (initialMaximum) -
                       static_cast<uint32_t> (initialMinimum) >=
                   config_.minimumSeparation;
    }

    bool CalibrationConsole::timeValid (TimePoint now) const noexcept
    {
        return !hasLastUpdate_ || now == lastUpdate_ ||
               now.elapsedSince (lastUpdate_).milliseconds () <= maximumDuration;
    }

    bool CalibrationConsole::acknowledgementDue (TimePoint now) const noexcept
    {
        return now.elapsedSince (stateSince_) >= config_.acknowledgement;
    }

    uint16_t CalibrationConsole::coarseCandidate (int16_t joystickY) const noexcept
    {
        int32_t normalized = joystickY;

        if (normalized < -1000)
        {
            normalized = -1000;
        }
        else if (normalized > 1000)
        {
            normalized = 1000;
        }

        const uint32_t low    = snapshot_.field == CalibrationField::Minimum
                                    ? config_.lowerLimit
                                    : static_cast<uint32_t> (snapshot_.previewMinimum) +
                                          config_.minimumSeparation;
        const uint32_t high   = snapshot_.field == CalibrationField::Minimum
                                    ? static_cast<uint32_t> (snapshot_.previewMaximum) -
                                          config_.minimumSeparation
                                    : config_.upperLimit;
        const uint32_t offset = static_cast<uint32_t> (normalized + 1000);
        const uint32_t value  = low + ((high - low) * offset + 1000u) / 2000u;

        return static_cast<uint16_t> (value);
    }

    uint16_t CalibrationConsole::clampCandidate (int32_t candidate) const noexcept
    {
        const int32_t low  = snapshot_.field == CalibrationField::Minimum
                                 ? config_.lowerLimit
                                 : static_cast<int32_t> (snapshot_.previewMinimum) +
                                       config_.minimumSeparation;
        const int32_t high = snapshot_.field == CalibrationField::Minimum
                                 ? static_cast<int32_t> (snapshot_.previewMaximum) -
                                       config_.minimumSeparation
                                 : config_.upperLimit;

        if (candidate < low)
        {
            candidate = low;
        }
        else if (candidate > high)
        {
            candidate = high;
        }

        return static_cast<uint16_t> (candidate);
    }

    void CalibrationConsole::beginEditing (TimePoint now) noexcept
    {
        snapshot_.previewMinimum = snapshot_.committedMinimum;
        snapshot_.previewMaximum = snapshot_.committedMaximum;
        snapshot_.changed        = false;
        snapshot_.state          = CalibrationConsoleState::Editing;
        stateSince_              = now;
    }

    void CalibrationConsole::commit (TimePoint now) noexcept
    {
        snapshot_.committedMinimum = snapshot_.previewMinimum;
        snapshot_.committedMaximum = snapshot_.previewMaximum;
        snapshot_.changed          = false;
        snapshot_.state            = CalibrationConsoleState::Committed;
        stateSince_                = now;
    }

    void CalibrationConsole::cancel (TimePoint now) noexcept
    {
        snapshot_.previewMinimum = snapshot_.committedMinimum;
        snapshot_.previewMaximum = snapshot_.committedMaximum;
        snapshot_.changed        = false;
        snapshot_.state          = CalibrationConsoleState::Cancelled;
        stateSince_              = now;
    }

    void CalibrationConsole::enterFault (Status status, TimePoint now) noexcept
    {
        snapshot_.previewMinimum = snapshot_.committedMinimum;
        snapshot_.previewMaximum = snapshot_.committedMaximum;
        snapshot_.changed        = false;
        snapshot_.state          = CalibrationConsoleState::Fault;
        snapshot_.status         = status;
        stateSince_              = now;
    }

    void CalibrationConsole::refreshChanged () noexcept
    {
        snapshot_.changed = snapshot_.previewMinimum != snapshot_.committedMinimum ||
                            snapshot_.previewMaximum != snapshot_.committedMaximum;
    }
} // namespace adk
