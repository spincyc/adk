#include "bounded_homing_policy.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        CopiedBinaryEvidence emptyEvidence (CarouselSourceKind kind) noexcept
        {
            return {{kind, 0, 0}, TimePoint (), 0, false, false, 0, StatusCode::Ok};
        }

        HomingSnapshot emptySnapshot (Status status) noexcept
        {
            return {HomingPhase::Uninitialized,
                    HomingFault::None,
                    0,
                    false,
                    0,
                    false,
                    0,
                    true,
                    0,
                    0,
                    0,
                    status};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validSourceKind (CarouselSourceKind kind) noexcept
        {
            return kind >= CarouselSourceKind::SyntheticIdentity &&
                   kind <= CarouselSourceKind::SyntheticStop;
        }

        bool sourceEqual (CarouselSource left, CarouselSource right) noexcept
        {
            return left.kind == right.kind && left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision;
        }

        bool evidenceEqual (const CopiedBinaryEvidence& left,
                            const CopiedBinaryEvidence& right) noexcept
        {
            return sourceEqual (left.source, right.source) &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence && left.active == right.active &&
                   left.qualified == right.qualified &&
                   left.qualificationEpoch == right.qualificationEpoch &&
                   left.status == right.status;
        }

        bool forwardOrEqual (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool due (TimePoint now, TimePoint deadline) noexcept
        {
            return forwardOrEqual (now, deadline);
        }

        TimePoint add (TimePoint point, Duration duration) noexcept
        {
            return TimePoint (point.milliseconds () + duration.milliseconds ());
        }

        bool withinSkew (TimePoint left, TimePoint right, Duration limit) noexcept
        {
            const uint32_t forward = left.elapsedSince (right).milliseconds ();
            const uint32_t reverse = right.elapsedSince (left).milliseconds ();
            const uint32_t delta   = forward < reverse ? forward : reverse;
            return delta < halfRange && delta <= limit.milliseconds ();
        }

        bool sourceShapeValid (CarouselSource     source,
                               CarouselSourceKind expected) noexcept
        {
            return validSourceKind (source.kind) && source.kind == expected &&
                   source.sourceId != 0 && source.configurationRevision != 0;
        }

        bool sequenceAdmissible (const CopiedBinaryEvidence& evidence,
                                 const CopiedBinaryEvidence& previous,
                                 bool                        hasPrevious) noexcept
        {
            if (!hasPrevious)
            {
                return true;
            }
            const uint32_t delta = evidence.sequence - previous.sequence;
            if (delta == 0)
            {
                return evidenceEqual (evidence, previous);
            }
            return delta < halfRange;
        }

        bool evidenceTimeValid (TimePoint now, const CopiedBinaryEvidence& evidence,
                                Duration maximumAge) noexcept
        {
            if (!forwardOrEqual (now, evidence.observedAt))
            {
                return false;
            }
            return now.elapsedSince (evidence.observedAt).milliseconds () <=
                   maximumAge.milliseconds ();
        }

        bool evidenceShapeValid (const CopiedBinaryEvidence& evidence,
                                 CarouselSourceKind          expected) noexcept
        {
            return sourceShapeValid (evidence.source, expected) &&
                   evidence.sequence != 0 && evidence.qualificationEpoch != 0 &&
                   validStatus (evidence.status);
        }

        int32_t stepped (int32_t position, int8_t direction) noexcept
        {
            return direction > 0 ? position + 1 : position - 1;
        }

        bool activeMotion (HomingPhase phase) noexcept
        {
            return phase == HomingPhase::SeekingHomeRelease ||
                   phase == HomingPhase::SeekingHome || phase == HomingPhase::Moving;
        }
    } // namespace

    HomingPreview::HomingPreview () noexcept
        // clang-format off
        : snapshot                   (emptySnapshot (StatusCode::NotInitialized)),
          status                     (StatusCode::NotInitialized),
          owner_                     (nullptr),
          lastHome_                  (emptyEvidence (CarouselSourceKind::SyntheticHome)),
          lastStop_                  (emptyEvidence (CarouselSourceKind::SyntheticStop)),
          lastUpdateAt_              (),
          phaseStartedAt_            (),
          nextStepAt_                (),
          targetLogicalPosition_     (0),
          generation_                (0),
          attemptQualificationEpoch_ (0),
          lastHomeEpoch_             (0),
          hasLastHome_               (false),
          hasLastStop_               (false),
          hasLastUpdate_             (false),
          hasNextStep_               (false),
          homeWasActive_             (false)
    // clang-format on
    {
    }

    BoundedHomingPolicy::BoundedHomingPolicy (
        const BoundedHomingConfig& config) noexcept
        // clang-format off
        : config_                     (config),
          snapshot_                   (emptySnapshot (StatusCode::NotInitialized)),
          lastHome_                   (emptyEvidence (CarouselSourceKind::SyntheticHome)),
          lastStop_                   (emptyEvidence (CarouselSourceKind::SyntheticStop)),
          lastUpdateAt_               (),
          phaseStartedAt_             (),
          nextStepAt_                 (),
          targetLogicalPosition_      (0),
          generation_                 (0),
          attemptQualificationEpoch_  (0),
          lastHomeEpoch_              (0),
          hasLastHome_                (false),
          hasLastStop_                (false),
          hasLastUpdate_              (false),
          hasNextStep_                (false),
          homeWasActive_              (false),
          initialized_                (false)
    // clang-format on
    {
    }

    void BoundedHomingPolicy::publishFault (HomingPreview& candidate, HomingFault fault,
                                            Status status) noexcept
    {
        candidate.snapshot.phase                  = HomingPhase::Fault;
        candidate.snapshot.fault                  = fault;
        candidate.snapshot.positionKnown          = false;
        candidate.snapshot.stepDirection          = 0;
        candidate.snapshot.stepRequested          = false;
        candidate.snapshot.requestedStepDirection = 0;
        candidate.snapshot.stopIntent             = true;
        candidate.snapshot.homeEpoch              = 0;
        candidate.snapshot.status                 = status;
        candidate.status                          = status;
        candidate.hasNextStep_                    = false;
    }

    Status BoundedHomingPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const uint32_t releaseDuration = config_.maximumReleaseDuration.milliseconds ();
        const uint32_t searchDuration  = config_.maximumSearchDuration.milliseconds ();
        const uint32_t interval        = config_.stepInterval.milliseconds ();
        const uint32_t evidenceAge     = config_.maximumEvidenceAge.milliseconds ();
        const uint32_t inputSkew       = config_.maximumInputSkew.milliseconds ();
        const int64_t releaseEnd = -static_cast<int64_t> (config_.homeSearchDirection) *
                                   config_.maximumReleaseSteps;
        const int64_t searchEnd  = static_cast<int64_t> (config_.homeSearchDirection) *
                                   config_.maximumSearchSteps;

        if (config_.minimumLogicalPosition >= 0 ||
            config_.maximumLogicalPosition <= 0 || config_.homeLogicalPosition != 0 ||
            (config_.homeSearchDirection != -1 && config_.homeSearchDirection != 1) ||
            config_.maximumReleaseSteps == 0 || config_.maximumSearchSteps == 0 ||
            releaseDuration == 0 || releaseDuration >= halfRange ||
            searchDuration == 0 || searchDuration >= halfRange || interval == 0 ||
            interval >= halfRange || evidenceAge == 0 || evidenceAge >= halfRange ||
            inputSkew >= halfRange || releaseEnd < INT32_MIN ||
            releaseEnd > INT32_MAX || searchEnd < INT32_MIN || searchEnd > INT32_MAX ||
            releaseEnd < config_.minimumLogicalPosition ||
            releaseEnd > config_.maximumLogicalPosition ||
            searchEnd < config_.minimumLogicalPosition ||
            searchEnd > config_.maximumLogicalPosition)
        {
            snapshot_       = emptySnapshot (StatusCode::InvalidConfiguration);
            snapshot_.fault = HomingFault::InvalidConfiguration;
            return snapshot_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void BoundedHomingPolicy::reset () noexcept
    {
        snapshot_ =
            emptySnapshot (initialized_ ? StatusCode::Ok : StatusCode::NotInitialized);
        snapshot_.phase =
            initialized_ ? HomingPhase::PositionUnknown : HomingPhase::Uninitialized;
        lastHome_                  = emptyEvidence (CarouselSourceKind::SyntheticHome);
        lastStop_                  = emptyEvidence (CarouselSourceKind::SyntheticStop);
        lastUpdateAt_              = TimePoint ();
        phaseStartedAt_            = TimePoint ();
        nextStepAt_                = TimePoint ();
        targetLogicalPosition_     = 0;
        attemptQualificationEpoch_ = 0;
        lastHomeEpoch_             = 0;
        hasLastHome_               = false;
        hasLastStop_               = false;
        hasLastUpdate_             = false;
        hasNextStep_               = false;
        homeWasActive_             = false;
        ++generation_;
    }

    void BoundedHomingPolicy::shutdown () noexcept
    {
        initialized_ = false;
        reset ();
        snapshot_.stopIntent = true;
    }

    Status BoundedHomingPolicy::preview (TimePoint now, const HomingInput& input,
                                         const HomingCommand& command,
                                         HomingPreview&       candidate) const noexcept
    {
        candidate.owner_                     = nullptr;
        candidate.snapshot                   = snapshot_;
        candidate.status                     = snapshot_.status;
        candidate.lastHome_                  = lastHome_;
        candidate.lastStop_                  = lastStop_;
        candidate.lastUpdateAt_              = lastUpdateAt_;
        candidate.phaseStartedAt_            = phaseStartedAt_;
        candidate.nextStepAt_                = nextStepAt_;
        candidate.targetLogicalPosition_     = targetLogicalPosition_;
        candidate.generation_                = generation_;
        candidate.attemptQualificationEpoch_ = attemptQualificationEpoch_;
        candidate.lastHomeEpoch_             = lastHomeEpoch_;
        candidate.hasLastHome_               = hasLastHome_;
        candidate.hasLastStop_               = hasLastStop_;
        candidate.hasLastUpdate_             = hasLastUpdate_;
        candidate.hasNextStep_               = hasNextStep_;
        candidate.homeWasActive_             = homeWasActive_;

        if (!initialized_)
        {
            candidate.snapshot = emptySnapshot (StatusCode::NotInitialized);
            candidate.status   = candidate.snapshot.status;
            return candidate.status;
        }

        if (!evidenceShapeValid (input.stop, CarouselSourceKind::SyntheticStop) ||
            !evidenceTimeValid (now, input.stop, config_.maximumEvidenceAge) ||
            !sequenceAdmissible (input.stop, lastStop_, hasLastStop_))
        {
            return StatusCode::InvalidArgument;
        }

        candidate.owner_         = this;
        candidate.lastStop_      = input.stop;
        candidate.hasLastStop_   = true;
        candidate.lastUpdateAt_  = now;
        candidate.hasLastUpdate_ = true;

        const bool qualifiedStop =
            input.stop.status.ok () && input.stop.qualified && input.stop.active;
        if (qualifiedStop)
        {
            if (snapshot_.phase == HomingPhase::Fault)
            {
                candidate.snapshot.stepDirection          = 0;
                candidate.snapshot.stepRequested          = false;
                candidate.snapshot.requestedStepDirection = 0;
                candidate.snapshot.stopIntent             = true;
                candidate.hasNextStep_                    = false;
                candidate.status                          = StatusCode::Ok;
                return StatusCode::Ok;
            }
            const bool interrupted   = activeMotion (snapshot_.phase);
            candidate.snapshot.phase = HomingPhase::Stopped;
            candidate.snapshot.fault =
                interrupted ? HomingFault::Interrupted : HomingFault::None;
            candidate.snapshot.positionKnown =
                interrupted ? false : snapshot_.positionKnown;
            candidate.snapshot.stepDirection          = 0;
            candidate.snapshot.stepRequested          = false;
            candidate.snapshot.requestedStepDirection = 0;
            candidate.snapshot.stopIntent             = true;
            candidate.snapshot.homingSteps            = 0;
            candidate.snapshot.homeEpoch = interrupted ? 0 : snapshot_.homeEpoch;
            candidate.snapshot.acceptedFrameSequence = input.frameSequence;
            candidate.snapshot.status                = StatusCode::Ok;
            candidate.status                         = StatusCode::Ok;
            candidate.hasNextStep_                   = false;
            return StatusCode::Ok;
        }

        if (!input.stop.status.ok () || !input.stop.qualified)
        {
            publishFault (candidate, HomingFault::EvidenceFault,
                          input.stop.status.ok () ? StatusCode::InvalidArgument
                                                  : input.stop.status);
            return candidate.status;
        }

        if (input.frameSequence == 0 || !forwardOrEqual (now, input.frameAt) ||
            now.elapsedSince (input.frameAt).milliseconds () >
                config_.maximumEvidenceAge.milliseconds () ||
            (hasLastUpdate_ && !forwardOrEqual (now, lastUpdateAt_)) ||
            !evidenceShapeValid (input.home, CarouselSourceKind::SyntheticHome) ||
            !evidenceTimeValid (now, input.home, config_.maximumEvidenceAge) ||
            !withinSkew (input.frameAt, input.home.observedAt,
                         config_.maximumInputSkew) ||
            !withinSkew (input.frameAt, input.stop.observedAt,
                         config_.maximumInputSkew) ||
            !withinSkew (input.home.observedAt, input.stop.observedAt,
                         config_.maximumInputSkew) ||
            !sequenceAdmissible (input.home, lastHome_, hasLastHome_) ||
            (command.requestHome && command.requestMove))
        {
            candidate.owner_ = nullptr;
            return StatusCode::InvalidArgument;
        }

        if (hasLastHome_ && input.frameSequence == snapshot_.acceptedFrameSequence &&
            !evidenceEqual (input.home, lastHome_))
        {
            candidate.owner_ = nullptr;
            return StatusCode::InvalidArgument;
        }
        if (snapshot_.acceptedFrameSequence != 0)
        {
            const uint32_t delta =
                input.frameSequence - snapshot_.acceptedFrameSequence;
            if (delta == 0 || delta >= halfRange)
            {
                candidate.owner_ = nullptr;
                return StatusCode::InvalidArgument;
            }
        }

        candidate.lastHome_                       = input.home;
        candidate.hasLastHome_                    = true;
        candidate.snapshot.stepRequested          = false;
        candidate.snapshot.requestedStepDirection = 0;
        candidate.snapshot.stopIntent             = false;
        candidate.snapshot.acceptedFrameSequence  = input.frameSequence;

        if (!input.home.status.ok () || !input.home.qualified)
        {
            publishFault (candidate, HomingFault::EvidenceFault,
                          input.home.status.ok () ? StatusCode::InvalidArgument
                                                  : input.home.status);
            return candidate.status;
        }

        if (snapshot_.phase == HomingPhase::Fault)
        {
            candidate.owner_ = nullptr;
            return StatusCode::InvalidArgument;
        }

        if (activeMotion (snapshot_.phase) &&
            (!sourceEqual (input.home.source, lastHome_.source) ||
             input.home.qualificationEpoch != attemptQualificationEpoch_))
        {
            publishFault (candidate, HomingFault::EvidenceFault,
                          StatusCode::InvalidArgument);
            return candidate.status;
        }

        if (command.requestHome)
        {
            if (activeMotion (snapshot_.phase))
            {
                candidate.owner_ = nullptr;
                return StatusCode::InvalidArgument;
            }
            candidate.attemptQualificationEpoch_ = input.home.qualificationEpoch;
            candidate.phaseStartedAt_            = now;
            candidate.nextStepAt_                = now;
            candidate.hasNextStep_               = true;
            candidate.homeWasActive_             = input.home.active;
            candidate.snapshot.phase             = input.home.active
                                                       ? HomingPhase::SeekingHomeRelease
                                                       : HomingPhase::SeekingHome;
            candidate.snapshot.fault             = HomingFault::None;
            candidate.snapshot.logicalPosition   = 0;
            candidate.snapshot.positionKnown     = false;
            candidate.snapshot.homeEpoch         = 0;
            candidate.snapshot.homingSteps       = 0;
        }

        HomingPhase phase = candidate.snapshot.phase;
        if (phase == HomingPhase::Stopped && !command.requestHome)
        {
            candidate.snapshot.stopIntent = true;
            return StatusCode::Ok;
        }

        if (phase == HomingPhase::PositionUnknown && !command.requestHome)
        {
            candidate.snapshot.stopIntent = true;
            return StatusCode::Ok;
        }

        if (phase == HomingPhase::Homed && command.requestMove)
        {
            if (command.targetLogicalPosition < config_.minimumLogicalPosition ||
                command.targetLogicalPosition > config_.maximumLogicalPosition)
            {
                candidate.owner_ = nullptr;
                return StatusCode::InvalidArgument;
            }
            candidate.targetLogicalPosition_ = command.targetLogicalPosition;
            if (command.targetLogicalPosition != snapshot_.logicalPosition)
            {
                candidate.snapshot.phase = HomingPhase::Moving;
                candidate.snapshot.stepDirection =
                    command.targetLogicalPosition > snapshot_.logicalPosition ? 1 : -1;
                candidate.nextStepAt_  = now;
                candidate.hasNextStep_ = true;
                phase                  = HomingPhase::Moving;
            }
        }

        if (phase == HomingPhase::SeekingHomeRelease)
        {
            const uint32_t elapsed =
                now.elapsedSince (candidate.phaseStartedAt_).milliseconds ();
            if (elapsed > config_.maximumReleaseDuration.milliseconds ())
            {
                publishFault (candidate, HomingFault::HomeStuckActive,
                              StatusCode::Timeout);
                return candidate.status;
            }
            if (!input.home.active)
            {
                candidate.snapshot.phase         = HomingPhase::SeekingHome;
                candidate.snapshot.homingSteps   = 0;
                candidate.phaseStartedAt_        = now;
                candidate.nextStepAt_            = now;
                candidate.hasNextStep_           = true;
                candidate.homeWasActive_         = false;
                candidate.snapshot.stepDirection = config_.homeSearchDirection;
                return StatusCode::Ok;
            }
            if (candidate.snapshot.homingSteps >= config_.maximumReleaseSteps &&
                due (now, candidate.nextStepAt_))
            {
                publishFault (candidate, HomingFault::HomeStuckActive,
                              StatusCode::Timeout);
                return candidate.status;
            }
            candidate.snapshot.stepDirection = -config_.homeSearchDirection;
        }
        else if (phase == HomingPhase::SeekingHome)
        {
            const uint32_t elapsed =
                now.elapsedSince (candidate.phaseStartedAt_).milliseconds ();
            if (elapsed > config_.maximumSearchDuration.milliseconds ())
            {
                publishFault (candidate, HomingFault::HomeNotFound,
                              StatusCode::Timeout);
                return candidate.status;
            }
            const bool acquired = !candidate.homeWasActive_ && input.home.active;
            if (acquired)
            {
                candidate.snapshot.phase                  = HomingPhase::Homed;
                candidate.snapshot.fault                  = HomingFault::None;
                candidate.snapshot.logicalPosition        = 0;
                candidate.snapshot.positionKnown          = true;
                candidate.snapshot.stepDirection          = 0;
                candidate.snapshot.stepRequested          = false;
                candidate.snapshot.requestedStepDirection = 0;
                candidate.snapshot.stopIntent             = true;
                candidate.snapshot.homingSteps            = 0;
                candidate.snapshot.homeEpoch = candidate.lastHomeEpoch_ + 1U;
                if (candidate.snapshot.homeEpoch == 0)
                {
                    candidate.snapshot.homeEpoch = 1;
                }
                candidate.lastHomeEpoch_ = candidate.snapshot.homeEpoch;
                candidate.hasNextStep_   = false;
                candidate.homeWasActive_ = true;
                return StatusCode::Ok;
            }
            if (candidate.snapshot.homingSteps >= config_.maximumSearchSteps &&
                due (now, candidate.nextStepAt_))
            {
                publishFault (candidate, HomingFault::HomeNotFound,
                              StatusCode::Timeout);
                return candidate.status;
            }
            candidate.snapshot.stepDirection = config_.homeSearchDirection;
            candidate.homeWasActive_         = input.home.active;
        }

        if ((phase == HomingPhase::SeekingHomeRelease ||
             phase == HomingPhase::SeekingHome) &&
            candidate.hasNextStep_ && due (now, candidate.nextStepAt_))
        {
            const int8_t direction = candidate.snapshot.stepDirection;
            if ((direction < 0 && candidate.snapshot.logicalPosition <=
                                      config_.minimumLogicalPosition) ||
                (direction > 0 &&
                 candidate.snapshot.logicalPosition >= config_.maximumLogicalPosition))
            {
                publishFault (candidate, HomingFault::TravelExceeded,
                              StatusCode::InvalidArgument);
                return candidate.status;
            }
            candidate.snapshot.logicalPosition =
                stepped (candidate.snapshot.logicalPosition, direction);
            candidate.snapshot.stepRequested          = true;
            candidate.snapshot.requestedStepDirection = direction;
            ++candidate.snapshot.homingSteps;
            candidate.nextStepAt_ = add (candidate.nextStepAt_, config_.stepInterval);
        }
        else if (phase == HomingPhase::Moving && candidate.hasNextStep_ &&
                 due (now, candidate.nextStepAt_))
        {
            const int8_t direction = candidate.snapshot.stepDirection;
            candidate.snapshot.logicalPosition =
                stepped (candidate.snapshot.logicalPosition, direction);
            candidate.snapshot.stepRequested          = true;
            candidate.snapshot.requestedStepDirection = direction;
            candidate.nextStepAt_ = add (candidate.nextStepAt_, config_.stepInterval);
            if (candidate.snapshot.logicalPosition == candidate.targetLogicalPosition_)
            {
                candidate.snapshot.phase         = HomingPhase::Homed;
                candidate.snapshot.stepDirection = 0;
                candidate.hasNextStep_           = false;
            }
        }

        candidate.snapshot.status = StatusCode::Ok;
        candidate.status          = StatusCode::Ok;
        return StatusCode::Ok;
    }

    bool BoundedHomingPolicy::canCommit (const HomingPreview& candidate) const noexcept
    {
        return initialized_ && candidate.owner_ == this &&
               candidate.generation_ == generation_;
    }

    Status BoundedHomingPolicy::commit (const HomingPreview& candidate) noexcept
    {
        if (!canCommit (candidate))
        {
            return StatusCode::InvalidArgument;
        }

        snapshot_                  = candidate.snapshot;
        lastHome_                  = candidate.lastHome_;
        lastStop_                  = candidate.lastStop_;
        lastUpdateAt_              = candidate.lastUpdateAt_;
        phaseStartedAt_            = candidate.phaseStartedAt_;
        nextStepAt_                = candidate.nextStepAt_;
        targetLogicalPosition_     = candidate.targetLogicalPosition_;
        attemptQualificationEpoch_ = candidate.attemptQualificationEpoch_;
        lastHomeEpoch_             = candidate.lastHomeEpoch_;
        hasLastHome_               = candidate.hasLastHome_;
        hasLastStop_               = candidate.hasLastStop_;
        hasLastUpdate_             = candidate.hasLastUpdate_;
        hasNextStep_               = candidate.hasNextStep_;
        homeWasActive_             = candidate.homeWasActive_;
        ++generation_;
        return candidate.status;
    }

    bool BoundedHomingPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    HomingSnapshot BoundedHomingPolicy::snapshot () const noexcept
    {
        return snapshot_;
    }

    HomingExcursionBounds BoundedHomingPolicy::excursionBounds () const noexcept
    {
        const int64_t release = -static_cast<int64_t> (config_.homeSearchDirection) *
                                config_.maximumReleaseSteps;
        const int64_t search  = static_cast<int64_t> (config_.homeSearchDirection) *
                                config_.maximumSearchSteps;
        const int64_t minimum = release < search ? release : search;
        const int64_t maximum = release > search ? release : search;
        return {static_cast<int32_t> (minimum < 0 ? minimum : 0),
                static_cast<int32_t> (maximum > 0 ? maximum : 0)};
    }
} // namespace adk
