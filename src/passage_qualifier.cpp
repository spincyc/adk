#include "passage_qualifier.h"

#include <limits.h>

// clang-format off
namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;


        PassagePositionEvidence emptyPosition () noexcept
        {
            return {false, false, false, 0, 0, 0};
        }

        PassageRecord emptyRecord () noexcept
        {
            return {0,
                    PassageDirection::Unknown,
                    PassageDisposition::EvidenceFault,
                    TimePoint       (),
                    TimePoint       (),
                    Duration        (),
                    MagneticPolarity::Unspecified,
                    MagneticPolarity::Unspecified,
                    emptyPosition (),
                    0,
                    0,
                    StatusCode::NotInitialized};
        }

        PassageSnapshot emptySnapshot () noexcept
        {
            return {PassagePhase::Idle,
                    PassageBoundary::None,
                    Duration (),
                    1,
                    0,
                    0,
                    false,
                    emptyRecord (),
                    StatusCode::NotInitialized};
        }

        bool validDuration (Duration value) noexcept
        {
            return value.milliseconds () != 0 && value.milliseconds () < halfRange;
        }

        bool healthy (const MagneticObservation& observation) noexcept
        {
            return observation.quality == MagneticQuality::Valid &&
                   observation.status.ok ();
        }

        uint32_t incrementSaturating (uint32_t value) noexcept
        {
            return value == UINT32_MAX ? value : value + 1;
        }

        MagneticPolarity polarityFor (const PassageInput& input,
                                      PassageBoundary     boundary) noexcept
        {
            return boundary == PassageBoundary::A ? input.boundaryA.polarity
                                                  : input.boundaryB.polarity;
        }

        bool sameObservation (const MagneticObservation& left,
                              const MagneticObservation& right) noexcept
        {
            return left.source == right.source && left.raw == right.raw &&
                   left.rawLevel == right.rawLevel &&
                   left.observedAt == right.observedAt &&
                   left.polarity == right.polarity &&
                   left.activationEvent == right.activationEvent &&
                   left.deactivationEvent == right.deactivationEvent &&
                   left.active == right.active && left.stableFor == right.stableFor &&
                   left.quality == right.quality && left.status == right.status;
        }

        bool sameInput (const PassageInput& left, const PassageInput& right) noexcept
        {
            return left.observedAt == right.observedAt &&
                   sameObservation (left.boundaryA, right.boundaryA) &&
                   sameObservation (left.boundaryB, right.boundaryB) &&
                   left.hasPosition == right.hasPosition &&
                   left.position == right.position &&
                   left.positionStatus == right.positionStatus;
        }
    } // namespace

    PassageQualifier::PassageQualifier (PassageQualifierConfig config) noexcept
        : config_                (config)
        , snapshot_              (emptySnapshot ())
        , lastUpdate_            ()
        , activeSinceA_          ()
        , activeSinceB_          ()
        , onset_                 ()
        , suppressionSince_      ()
        , lastInput_             ()
        , onsetPosition_         (0)
        , onsetPositionStatus_   (StatusCode::NotInitialized)
        , onsetPolarity_         (MagneticPolarity::Unspecified)
        , onsetHasPosition_      (false)
        , hasUpdate_             (false)
        , activeA_               (false)
        , activeB_               (false)
        , qualifiedA_            (false)
        , qualifiedB_            (false)
        , retreating_            (false)
        , suppressionActivation_ (false)
        , initialized_           (false)
    {
    }

    Status PassageQualifier::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validDuration (config_.boundaryDwell) ||
            !validDuration (config_.passageTimeout) ||
            !validDuration (config_.duplicateWindow) ||
            config_.passageTimeout < config_.boundaryDwell)
        {
            snapshot_.status = StatusCode::InvalidConfiguration;
            return snapshot_.status;
        }

        initialized_ = true;

        reset ();
        return StatusCode::Ok;
    }

    void PassageQualifier::reset () noexcept
    {
        snapshot_ = emptySnapshot ();

        clearActivity ();
        hasUpdate_             = false;
        suppressionActivation_ = false;

        lastUpdate_            = TimePoint ();

        activeSinceA_          = TimePoint ();

        activeSinceB_          = TimePoint ();

        onset_                 = TimePoint ();

        suppressionSince_      = TimePoint ();

        lastInput_             = PassageInput ();
        onsetPosition_         = 0;
        onsetPositionStatus_   = StatusCode::NotInitialized;
        onsetPolarity_         = MagneticPolarity::Unspecified;
        onsetHasPosition_      = false;


        if (initialized_)
        {
            snapshot_.status = StatusCode::Ok;
        }
    }

    void PassageQualifier::update (const PassageInput& input) noexcept
    {
        if (!initialized_)
        {
            snapshot_.status = StatusCode::NotInitialized;
            return;
        }

        if (hasUpdate_)
        {
            const Duration elapsed = input.observedAt.elapsedSince (lastUpdate_);


            if (elapsed.milliseconds () >= halfRange)
            {
                enterFault (input, StatusCode::InvalidArgument);
                return;
            }

            if (elapsed == Duration ())
            {
                if (sameInput (input, lastInput_))
                {
                    return;
                }

                enterFault (input, StatusCode::InvalidArgument);
                return;
            }
        }

        snapshot_.hasRecord = false;


        if (input.boundaryA.observedAt != input.observedAt ||
            input.boundaryB.observedAt != input.observedAt)
        {
            enterFault (input, StatusCode::InvalidArgument);
            return;
        }

        if (!healthy (input.boundaryA) || !healthy (input.boundaryB))
        {
            enterFault (input, StatusCode::HardwareFailure);
            return;
        }

        const bool wasActiveA = activeA_;
        const bool wasActiveB = activeB_;
        activeA_              = input.boundaryA.active;
        activeB_              = input.boundaryB.active;


        if (activeA_ && !wasActiveA)
        {
            activeSinceA_ = input.observedAt;
        }

        if (activeB_ && !wasActiveB)
        {
            activeSinceB_ = input.observedAt;
        }

        qualifiedA_ = activeA_ && input.observedAt.elapsedSince (activeSinceA_) >=
                                      config_.boundaryDwell;

        qualifiedB_ = activeB_ && input.observedAt.elapsedSince (activeSinceB_) >=
                                      config_.boundaryDwell;

        lastUpdate_ = input.observedAt;
        lastInput_  = input;
        hasUpdate_  = true;


        if (snapshot_.phase == PassagePhase::Fault)
        {
            if (!activeA_ && !activeB_)
            {
                snapshot_.phase         = PassagePhase::Idle;
                snapshot_.firstBoundary = PassageBoundary::None;
                snapshot_.status        = StatusCode::Ok;

                clearActivity ();
                hasUpdate_  = true;
                lastUpdate_ = input.observedAt;
            }

            return;
        }

        if (snapshot_.phase == PassagePhase::Suppressing)
        {
            if ((activeA_ || activeB_) && !suppressionActivation_)
            {
                suppressionActivation_ = true;
                snapshot_.suppressedCount =
                    incrementSaturating (snapshot_.suppressedCount);
                onset_ = input.observedAt;
                onsetPolarity_ =
                    activeA_ ? input.boundaryA.polarity : input.boundaryB.polarity;
                onsetPosition_ = input.hasPosition ? input.position : 0;

                emit (PassageDisposition::DuplicateSuppressed,
                      PassageDirection::Unknown, input.observedAt, onsetPolarity_,
                      StatusCode::Ok);
            }

            if (!activeA_ && !activeB_)
            {
                suppressionActivation_ = false;


                if (input.observedAt.elapsedSince (suppressionSince_) >=
                    config_.duplicateWindow)
                {
                    snapshot_.phase         = PassagePhase::Idle;
                    snapshot_.firstBoundary = PassageBoundary::None;
                    snapshot_.status        = StatusCode::Ok;

                    clearActivity ();
                    hasUpdate_  = true;
                    lastUpdate_ = input.observedAt;
                }
            }

            return;
        }

        if (snapshot_.phase == PassagePhase::Idle)
        {
            if (qualifiedA_ && qualifiedB_)
            {
                onset_         = input.observedAt;
                onsetPolarity_ = input.boundaryA.polarity;
                onsetPosition_ = input.hasPosition ? input.position : 0;

                emit (PassageDisposition::Ambiguous, PassageDirection::Unknown,
                      input.observedAt, input.boundaryB.polarity,
                      StatusCode::InvalidArgument);
                return;
            }

            if (activeA_ || activeB_)
            {
                snapshot_.phase = PassagePhase::FirstBoundary;
                onset_          = input.observedAt;
                onsetPolarity_ =
                    activeA_ ? input.boundaryA.polarity : input.boundaryB.polarity;
                onsetPosition_       = input.hasPosition ? input.position : 0;
                onsetPositionStatus_ = input.positionStatus;
                onsetHasPosition_    = input.hasPosition;
            }

            if (qualifiedA_ || qualifiedB_)
            {
                snapshot_.firstBoundary =
                    qualifiedA_ ? PassageBoundary::A : PassageBoundary::B;
                snapshot_.phase      = PassagePhase::AwaitingSecond;
                onset_               = input.observedAt;

                onsetPolarity_       = polarityFor (input, snapshot_.firstBoundary);
                onsetPosition_       = input.hasPosition ? input.position : 0;
                onsetPositionStatus_ = input.positionStatus;
                onsetHasPosition_    = input.hasPosition;
            }

            return;
        }

        if (snapshot_.phase == PassagePhase::FirstBoundary)
        {
            if (!activeA_ && !activeB_)
            {
                snapshot_.phase = PassagePhase::Idle;
                return;
            }

            if (qualifiedA_ && qualifiedB_)
            {
                onset_         = input.observedAt;
                onsetPolarity_ = input.boundaryA.polarity;
                onsetPosition_ = input.hasPosition ? input.position : 0;

                emit (PassageDisposition::Ambiguous, PassageDirection::Unknown,
                      input.observedAt, input.boundaryB.polarity,
                      StatusCode::InvalidArgument);
                return;
            }

            if (qualifiedA_ || qualifiedB_)
            {
                snapshot_.firstBoundary =
                    qualifiedA_ ? PassageBoundary::A : PassageBoundary::B;
                snapshot_.phase      = PassagePhase::AwaitingSecond;
                onset_               = input.observedAt;

                onsetPolarity_       = polarityFor (input, snapshot_.firstBoundary);
                onsetPosition_       = input.hasPosition ? input.position : 0;
                onsetPositionStatus_ = input.positionStatus;
                onsetHasPosition_    = input.hasPosition;
            }

            return;
        }

        snapshot_.elapsed = input.observedAt.elapsedSince (onset_);

        const bool firstActive =
            snapshot_.firstBoundary == PassageBoundary::A ? activeA_ : activeB_;
        const bool oppositeQualified =
            snapshot_.firstBoundary == PassageBoundary::A ? qualifiedB_ : qualifiedA_;


        if (!firstActive)
        {
            retreating_ = true;
        }

        if (retreating_)
        {
            if (!activeA_ && !activeB_)
            {
                snapshot_.phase         = PassagePhase::Idle;
                snapshot_.firstBoundary = PassageBoundary::None;

                snapshot_.elapsed       = Duration ();

                clearActivity ();
                hasUpdate_  = true;
                lastUpdate_ = input.observedAt;
            }

            return;
        }

        if (snapshot_.elapsed > config_.passageTimeout)
        {
            emit (PassageDisposition::TimedOut, PassageDirection::Unknown,
                  input.observedAt, MagneticPolarity::Unspecified, StatusCode::Timeout);
            return;
        }

        if (oppositeQualified)
        {
            const PassageDirection direction =
                snapshot_.firstBoundary == PassageBoundary::A ? PassageDirection::AToB
                                                              : PassageDirection::BToA;
            const MagneticPolarity endPolarity =
                snapshot_.firstBoundary == PassageBoundary::A
                    ? input.boundaryB.polarity
                    : input.boundaryA.polarity;


            updatePosition (input);

            snapshot_.acceptedCount = incrementSaturating (snapshot_.acceptedCount);

            emit (PassageDisposition::Accepted, direction, input.observedAt,
                  endPolarity, StatusCode::Ok);
        }
    }

    PassageSnapshot PassageQualifier::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool PassageQualifier::initialized () const noexcept
    {
        return initialized_;
    }

    void PassageQualifier::clearActivity () noexcept
    {
        activeA_          = false;
        activeB_          = false;
        qualifiedA_       = false;
        qualifiedB_       = false;
        retreating_       = false;

        snapshot_.elapsed = Duration ();
    }

    void PassageQualifier::enterFault (const PassageInput& input,
                                       Status              status) noexcept
    {
        const bool candidate = snapshot_.phase == PassagePhase::FirstBoundary ||
                               snapshot_.phase == PassagePhase::AwaitingSecond;


        if (candidate)
        {
            emit (PassageDisposition::EvidenceFault, PassageDirection::Unknown,
                  input.observedAt, MagneticPolarity::Unspecified, status);
        }

        snapshot_.phase  = PassagePhase::Fault;
        snapshot_.status = status;
    }

    void PassageQualifier::emit (PassageDisposition disposition,
                                 PassageDirection direction, TimePoint end,
                                 MagneticPolarity endPolarity, Status status) noexcept
    {
        const PassagePositionEvidence position =
            disposition == PassageDisposition::Accepted ? snapshot_.record.position
                                                        : emptyPosition ();

        snapshot_.record       = {snapshot_.nextSequence,
                                  direction,
                                  disposition,
                                  onset_,
                                  end,
                                  end.elapsedSince (onset_),
                                  onsetPolarity_,
                                  endPolarity,
                                  position,
                                  snapshot_.acceptedCount,
                                  snapshot_.suppressedCount,
                                  status};

        snapshot_.nextSequence = incrementSaturating (snapshot_.nextSequence);
        snapshot_.hasRecord    = true;
        snapshot_.status       = status;


        if (disposition == PassageDisposition::EvidenceFault)
        {
            snapshot_.phase = PassagePhase::Fault;
        }
        else
        {
            snapshot_.phase         = PassagePhase::Suppressing;
            snapshot_.firstBoundary = PassageBoundary::None;

            if (disposition != PassageDisposition::DuplicateSuppressed)
            {
                suppressionSince_ = end;
            }
            suppressionActivation_ = activeA_ || activeB_;
        }
    }

    void PassageQualifier::updatePosition (const PassageInput& input) noexcept
    {
        PassagePositionEvidence& position = snapshot_.record.position;


        if (!onsetHasPosition_ || !input.hasPosition)
        {
            position = emptyPosition ();
            return;
        }

        position.present       = true;
        position.onsetPosition = onsetPosition_;
        position.endPosition   = input.position;


        const int64_t difference = static_cast<int64_t> (input.position) -
                                   static_cast<int64_t> (onsetPosition_);


        if (difference > INT32_MAX)
        {
            position.delta     = INT32_MAX;
            position.saturated = true;
        }
        else if (difference < INT32_MIN)
        {
            position.delta     = INT32_MIN;
            position.saturated = true;
        }
        else
        {
            position.delta     = static_cast<int32_t> (difference);
            position.saturated = false;
        }

        const bool directionAgrees =
            (snapshot_.firstBoundary == PassageBoundary::A && position.delta > 0) ||
            (snapshot_.firstBoundary == PassageBoundary::B && position.delta < 0);

        position.reliable = onsetPositionStatus_.ok () && input.positionStatus.ok () &&
                            !position.saturated && position.delta != 0 &&
                            directionAgrees;
    }
} // namespace adk
// clang-format on
