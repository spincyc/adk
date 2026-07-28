#include "analog_joystick.h"

#include "board.h"

namespace adk {

    constexpr int16_t AnalogJoystick::minimumPosition;
    constexpr int16_t AnalogJoystick::maximumPosition;

    JoystickAxisConfig::JoystickAxisConfig (
        PinId    pin,
        uint16_t center,
        uint16_t observedMinimum,
        uint16_t observedMaximum,
        uint16_t deadZone,
        bool     inverted) noexcept
        : pin             (pin)
        , center          (center)
        , observedMinimum (observedMinimum)
        , observedMaximum (observedMaximum)
        , deadZone        (deadZone)
        , inverted        (inverted)
    {
    }

    AnalogJoystickConfig::AnalogJoystickConfig (
        const JoystickAxisConfig& xAxis,
        const JoystickAxisConfig& yAxis,
        const ButtonConfig&       selectButton) noexcept
        : xAxis        (xAxis)
        , yAxis        (yAxis)
        , selectButton (selectButton)
    {
    }

    AnalogJoystick::AnalogJoystick (
        ResourceRegistry&           resources,
        const AnalogJoystickConfig& config) noexcept
        : config_       (config)
        , xInput_       (resources, config.xAxis.pin)
        , yInput_       (resources, config.yAxis.pin)
        , selectButton_ (resources, config.selectButton)
        , snapshot_     ({{0, 0, true, false},
                          {0, 0, true, false},
                          false,
                          false,
                          false,
                          false,
                          StatusCode::NotInitialized})
        , initialized_  (false)
    {
    }

    AnalogJoystick::~AnalogJoystick () noexcept
    {
        shutdown ();
    }

    Status AnalogJoystick::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (config_.xAxis.pin)
            || !Mega2560Board::validPin (config_.yAxis.pin)
            || !Mega2560Board::validPin (config_.selectButton.pin))
        {
            setStatus (StatusCode::InvalidPin);
            return StatusCode::InvalidPin;
        }

        if (!Mega2560Board::supports (
                config_.xAxis.pin,
                PinCapability::AnalogInput)
            || !Mega2560Board::supports (
                config_.yAxis.pin,
                PinCapability::AnalogInput)
            || !Mega2560Board::supports (
                config_.selectButton.pin,
                PinCapability::DigitalInput))
        {
            setStatus (StatusCode::Unsupported);
            return StatusCode::Unsupported;
        }

        if (config_.xAxis.pin == config_.yAxis.pin
            || config_.xAxis.pin == config_.selectButton.pin
            || config_.yAxis.pin == config_.selectButton.pin
            || !validAxis (config_.xAxis)
            || !validAxis (config_.yAxis))
        {
            setStatus (StatusCode::InvalidConfiguration);
            return StatusCode::InvalidConfiguration;
        }

        Status status = xInput_.initialize ();

        if (!status.ok ())
        {
            setStatus (status);
            return status;
        }

        status = yInput_.initialize ();

        if (!status.ok ())
        {
            xInput_.shutdown ();

            setStatus (status);
            return status;
        }

        status = selectButton_.initialize ();

        if (!status.ok ())
        {
            yInput_.shutdown ();
            xInput_.shutdown ();

            setStatus (status);
            return status;
        }

        initialized_ = true;
        snapshot_.x  = deriveAxis (xInput_.read (), config_.xAxis);
        snapshot_.y  = deriveAxis (yInput_.read (), config_.yAxis);

        snapshot_.rawSelected = selectButton_.rawPressed ();
        snapshot_.selected    = selectButton_.pressed    ();
        snapshot_.selectEvent = false;
        snapshot_.releaseEvent = false;

        setStatus (StatusCode::Ok);
        return StatusCode::Ok;
    }

    void AnalogJoystick::shutdown () noexcept
    {
        if (initialized_)
        {
            selectButton_.shutdown ();
            yInput_.shutdown       ();
            xInput_.shutdown       ();
            initialized_ = false;
        }

        snapshot_.selectEvent  = false;
        snapshot_.releaseEvent = false;
        setStatus (StatusCode::NotInitialized);
    }

    Status AnalogJoystick::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            setStatus (StatusCode::NotInitialized);
            return StatusCode::NotInitialized;
        }

        xInput_.update       ();
        yInput_.update       ();
        selectButton_.update (now);

        snapshot_.x            = deriveAxis (xInput_.read (), config_.xAxis);
        snapshot_.y            = deriveAxis (yInput_.read (), config_.yAxis);

        snapshot_.rawSelected  = selectButton_.rawPressed   ();
        snapshot_.selected     = selectButton_.pressed      ();
        snapshot_.selectEvent  = selectButton_.pressEvent   ();
        snapshot_.releaseEvent = selectButton_.releaseEvent ();

        setStatus (StatusCode::Ok);
        return StatusCode::Ok;
    }

    bool AnalogJoystick::initialized () const noexcept
    {
        return initialized_;
    }

    AnalogJoystickSnapshot AnalogJoystick::snapshot () const noexcept
    {
        return snapshot_;
    }

    const AnalogInput& AnalogJoystick::xInput () const noexcept
    {
        return xInput_;
    }

    const AnalogInput& AnalogJoystick::yInput () const noexcept
    {
        return yInput_;
    }

    const Button& AnalogJoystick::selectButton () const noexcept
    {
        return selectButton_;
    }

    bool AnalogJoystick::validAxis (
        const JoystickAxisConfig& axis) noexcept
    {
        if (axis.observedMinimum > AnalogInput::maximumReading
            || axis.observedMaximum > AnalogInput::maximumReading
            || axis.center > AnalogInput::maximumReading
            || axis.observedMinimum >= axis.observedMaximum
            || axis.center <= axis.observedMinimum
            || axis.center >= axis.observedMaximum)
        {
            return false;
        }

        const uint32_t lowerEdge = static_cast<uint32_t> (axis.center)
            - axis.deadZone;
        const uint32_t upperEdge = static_cast<uint32_t> (axis.center)
            + axis.deadZone;

        return axis.deadZone <= axis.center
            && lowerEdge > axis.observedMinimum
            && upperEdge < axis.observedMaximum;
    }

    JoystickAxisSnapshot AnalogJoystick::deriveAxis (
        uint16_t                  raw,
        const JoystickAxisConfig& config) const noexcept
    {
        const uint16_t lowerEdge = static_cast<uint16_t> (
            config.center - config.deadZone);
        const uint16_t upperEdge = static_cast<uint16_t> (
            config.center + config.deadZone);
        const uint16_t bounded = raw < config.observedMinimum
            ? config.observedMinimum
            : (raw > config.observedMaximum
                   ? config.observedMaximum
                   : raw);

        int32_t position = 0;

        if (bounded < lowerEdge)
        {
            const uint32_t span = static_cast<uint32_t> (
                lowerEdge - config.observedMinimum);
            const uint32_t distance = static_cast<uint32_t> (
                lowerEdge - bounded);
            position = -static_cast<int32_t> (
                distance * static_cast<uint32_t> (maximumPosition) / span);
        }
        else if (bounded > upperEdge)
        {
            const uint32_t span = static_cast<uint32_t> (
                config.observedMaximum - upperEdge);
            const uint32_t distance = static_cast<uint32_t> (
                bounded - upperEdge);
            position = static_cast<int32_t> (
                distance * static_cast<uint32_t> (maximumPosition) / span);
        }

        if (config.inverted)
        {
            position = -position;
        }

        return {
            raw,
            static_cast<int16_t> (position),
            raw >= lowerEdge && raw <= upperEdge,
            raw <= config.observedMinimum || raw >= config.observedMaximum};
    }

    void AnalogJoystick::setStatus (Status status) noexcept
    {
        snapshot_.status = status;
    }
}
