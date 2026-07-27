#include "button.h"

namespace adk {

    ButtonConfig::ButtonConfig (PinId    pin,
                                Pull     pull,
                                Level    pressedLevel,
                                Duration debounce) noexcept
        : pin          (pin)
        , pull         (pull)
        , pressedLevel (pressedLevel)
        , debounce     (debounce)
    {
    }

    Button::Button (ResourceRegistry& resources,
                    const ButtonConfig& config) noexcept
        : input_              (resources, config.pin, config.pull)
        , pressedLevel_       (config.pressedLevel)
        , debounce_           (config.debounce)
        , candidateSince_     (TimePoint (0))
        , candidatePressed_   (false)
        , stablePressed_      (false)
        , pressEvent_         (false)
        , releaseEvent_       (false)
        , pressArmed_         (false)
        , timingCandidate_    (false)
    {
    }

    Button::~Button () noexcept
    {
        shutdown ();
    }

    Status Button::initialize () noexcept
    {
        const Status status = input_.initialize ();

        if (status != Status::Ok)
        {
            return status;
        }

        input_.update                      ();
        candidatePressed_ = levelIsPressed (input_.read ());
        stablePressed_    = candidatePressed_;
        pressEvent_       = false;
        releaseEvent_     = false;
        pressArmed_       = !stablePressed_;
        timingCandidate_  = false;

        return Status::Ok;
    }

    void Button::shutdown () noexcept
    {
        input_.shutdown ();
        candidatePressed_ = false;
        stablePressed_    = false;
        pressEvent_       = false;
        releaseEvent_     = false;
        pressArmed_       = false;
        timingCandidate_  = false;
    }

    void Button::update (TimePoint now) noexcept
    {
        pressEvent_   = false;
        releaseEvent_ = false;

        if (!input_.initialized ())
        {
            return;
        }

        input_.update                              ();
        const bool sampledPressed = levelIsPressed (input_.read ());

        if (sampledPressed != candidatePressed_)
        {
            candidatePressed_ = sampledPressed;
            candidateSince_   = now;
            timingCandidate_  = true;
        }

        if (!timingCandidate_ || candidatePressed_ == stablePressed_)
        {
            return;
        }

        if (now.elapsedSince (candidateSince_).milliseconds () <
            debounce_.milliseconds ())
        {
            return;
        }

        stablePressed_   = candidatePressed_;
        timingCandidate_ = false;

        if (!stablePressed_)
        {
            releaseEvent_ = true;
            pressArmed_    = true;
            return;
        }

        if (pressArmed_)
        {
            pressEvent_ = true;
            pressArmed_ = false;
        }
    }

    bool Button::initialized () const noexcept
    {
        return input_.initialized ();
    }

    bool Button::rawPressed () const noexcept
    {
        return candidatePressed_;
    }

    bool Button::pressed () const noexcept
    {
        return stablePressed_;
    }

    bool Button::pressEvent () const noexcept
    {
        return pressEvent_;
    }

    bool Button::releaseEvent () const noexcept
    {
        return releaseEvent_;
    }

    const DigitalInput& Button::input () const noexcept
    {
        return input_;
    }

    bool Button::levelIsPressed (Level level) const noexcept
    {
        return level == pressedLevel_;
    }
}
