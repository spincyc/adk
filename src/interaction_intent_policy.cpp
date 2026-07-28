#include "interaction_intent_policy.h"

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange                = 0x80000000UL;
        constexpr uint16_t maximumMagnitudePermille = 1000;

        InteractionSource emptySource () noexcept
        {
            return {InteractionSourceKind::SyntheticFixture, 0, 0};
        }

        InteractionIntent emptyIntent (Status status) noexcept
        {
            return {emptySource        (),
                    emptySource        (),
                    TimePoint          (),
                    0,
                    0,
                    InteractionDirection::Neutral,
                    0,
                    false,
                    false,
                    false,
                    false,
                    InteractionQuality::Invalid,
                    Duration (),
                    Duration (),
                    false,
                    ContactQuality::Unqualified,
                    status,
                    status,
                    status};
        }

        bool validDuration (Duration duration) noexcept
        {
            return duration.milliseconds () != 0 &&
                   duration.milliseconds () < halfRange;
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validKind (InteractionSourceKind kind) noexcept
        {
            return kind >= InteractionSourceKind::SyntheticFixture &&
                   kind <= InteractionSourceKind::CopiedJoystick;
        }

        bool validSource (const InteractionSource& source) noexcept
        {
            return validKind (source.kind) && source.sourceId != 0 &&
                   source.configurationRevision != 0;
        }

        bool validContactSource (const InteractionSource& source) noexcept
        {
            return validSource (source) &&
                   (source.kind == InteractionSourceKind::SyntheticFixture ||
                    source.kind == InteractionSourceKind::CopiedContact);
        }

        bool validDirectionalSource (const InteractionSource& source) noexcept
        {
            return validSource (source) &&
                   (source.kind == InteractionSourceKind::SyntheticFixture ||
                    source.kind == InteractionSourceKind::CopiedJoystick);
        }

        bool sameSource (const InteractionSource& left,
                         const InteractionSource& right) noexcept
        {
            return left.kind == right.kind && left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision;
        }

        bool sameContact (const ContactSample& left,
                          const ContactSample& right) noexcept
        {
            return left.observedAt == right.observedAt &&
                   left.rawLevel == right.rawLevel && left.status == right.status;
        }

        bool sameDirectional (const DirectionalEvidence& left,
                              const DirectionalEvidence& right) noexcept
        {
            return sameSource (left.source, right.source) &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence &&
                   left.xPermille == right.xPermille &&
                   left.yPermille == right.yPermille &&
                   left.saturated == right.saturated && left.status == right.status;
        }

        bool validLevel (Level level) noexcept
        {
            return level == Level::Low || level == Level::High;
        }

        bool forwardOrRepeat (uint32_t current, uint32_t previous) noexcept
        {
            return current - previous < halfRange;
        }

        bool timeForwardOrRepeat (TimePoint current, TimePoint previous) noexcept
        {
            return current.elapsedSince (previous).milliseconds () < halfRange;
        }

        uint16_t absolute (int16_t value) noexcept
        {
            const int32_t widened = value;
            return static_cast<uint16_t> (widened < 0 ? -widened : widened);
        }

        InteractionDirection directionFor (int16_t x, int16_t y) noexcept
        {
            const uint16_t ax        = absolute (x);
            const uint16_t ay        = absolute (y);
            const uint16_t major     = ax > ay ? ax : ay;
            const uint16_t minor     = ax > ay ? ay : ax;
            const bool     principal = static_cast<uint32_t> (minor) * 1000U <=
                                       static_cast<uint32_t> (major) * 414U;

            if (principal)
            {
                if (ax > ay)
                {
                    return x > 0 ? InteractionDirection::East
                                 : InteractionDirection::West;
                }
                return y > 0 ? InteractionDirection::North
                             : InteractionDirection::South;
            }

            if (y > 0)
            {
                return x > 0 ? InteractionDirection::NorthEast
                             : InteractionDirection::NorthWest;
            }
            return x > 0 ? InteractionDirection::SouthEast
                         : InteractionDirection::SouthWest;
        }

        InteractionQuality qualityFor (const ContactObservation& contact,
                                       Status directionalStatus, bool stale) noexcept
        {
            if (contact.quality == ContactQuality::TimingFault)
            {
                return InteractionQuality::TimingFault;
            }
            if (contact.quality == ContactQuality::SourceFault ||
                !directionalStatus.ok ())
            {
                return InteractionQuality::SourceFault;
            }
            if (contact.quality == ContactQuality::StuckActive)
            {
                return InteractionQuality::StuckActive;
            }
            return stale ? InteractionQuality::Stale : InteractionQuality::Current;
        }
    } // namespace

    InteractionIntentConfig::InteractionIntentConfig (
        const ContactDynamicsConfig& contactValue, Duration maximumContactAgeValue,
        Duration maximumDirectionalAgeValue, uint16_t engageMagnitudePermilleValue,
        uint16_t releaseMagnitudePermilleValue) noexcept
        : contact                   (contactValue),
          maximumContactAge         (maximumContactAgeValue),
          maximumDirectionalAge     (maximumDirectionalAgeValue),
          engageMagnitudePermille   (engageMagnitudePermilleValue),
          releaseMagnitudePermille  (releaseMagnitudePermilleValue)
    {
    }

    InteractionIntentPreview::InteractionIntentPreview () noexcept
        : owner                    (nullptr),
          generation               (0),
          now                      (),
          contactSource            (emptySource ()),
          contactSequence          (0),
          contact                  (),
          directional              (),
          direction                (InteractionDirection::Neutral),
          magnitudePermille        (0),
          directionEvent           (false),
          contactRepeat            (false),
          directionalRepeat        (false),
          contactDomainChanged     (false),
          directionalDomainChanged (false),
          contactRecovery          (false),
          recoveryBaseline         (false)
    {
    }

    InteractionIntentPolicy::InteractionIntentPolicy (
        const InteractionIntentConfig& config) noexcept
        : contact_                (config.contact),
          config_                 (config),
          intent_                 (emptyIntent (StatusCode::NotInitialized)),
          lastContactSource_      (emptySource ()),
          lastDirectionalSource_  (emptySource ()),
          lastContact_            (),
          lastDirectional_        (),
          lastContactSequence_    (0),
          generation_             (0),
          initialized_            (false),
          hasContact_             (false),
          hasDirectional_         (false)
    {
    }

    Status InteractionIntentPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validDuration (config_.maximumContactAge) ||
            !validDuration (config_.maximumDirectionalAge) ||
            config_.engageMagnitudePermille == 0 ||
            config_.engageMagnitudePermille > maximumMagnitudePermille ||
            config_.releaseMagnitudePermille > config_.engageMagnitudePermille)
        {
            intent_ = emptyIntent (StatusCode::InvalidConfiguration);
            return intent_.status;
        }

        const Status status = contact_.initialize ();

        if (!status.ok      ())
        {
            intent_ = emptyIntent (status);
            return status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void InteractionIntentPolicy::reset () noexcept
    {
        contact_.reset ();
        intent_ =
            emptyIntent (initialized_ ? StatusCode::Ok : StatusCode::NotInitialized);
        lastContactSource_     = emptySource         ();
        lastDirectionalSource_ = emptySource         ();
        lastContact_           = ContactSample       ();
        lastDirectional_       = DirectionalEvidence ();
        lastContactSequence_   = 0;
        hasContact_            = false;
        hasDirectional_        = false;
        ++generation_;
    }

    Status InteractionIntentPolicy::preview (
        TimePoint now, const InteractionSource& contactSource, uint32_t contactSequence,
        const ContactSample& contact, const DirectionalEvidence& directional,
        InteractionIntentPreview& candidate) const noexcept
    {
        candidate.owner      = nullptr;
        candidate.generation = 0;

        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!validContactSource     (contactSource) ||
            !validDirectionalSource (directional.source) ||
            !validStatus            (contact.status) ||
            !validStatus            (directional.status) ||
            !validLevel             (contact.rawLevel) ||
            directional.xPermille < -1000 ||
            directional.xPermille > 1000 || directional.yPermille < -1000 ||
            directional.yPermille > 1000 ||
            ((hasContact_ || hasDirectional_) &&
             !timeForwardOrRepeat (now, intent_.observedAt)))
        {
            return StatusCode::InvalidArgument;
        }

        const Duration contactAge     = now.elapsedSince (contact.observedAt);
        const Duration directionalAge = now.elapsedSince (directional.observedAt);

        if (contactAge.milliseconds     () >= halfRange ||
            directionalAge.milliseconds () >= halfRange)
        {
            return StatusCode::InvalidArgument;
        }

        const bool contactDomainChanged =
            hasContact_ && !sameSource (contactSource, lastContactSource_);
        const bool directionalDomainChanged =
            hasDirectional_ && !sameSource (directional.source, lastDirectionalSource_);
        const bool contactRepeat = hasContact_ && !contactDomainChanged &&
                                   contactSequence == lastContactSequence_;
        const bool directionalRepeat =
            hasDirectional_ && !directionalDomainChanged &&
            directional.sequence == lastDirectional_.sequence;

        if (hasContact_ && !contactDomainChanged)
        {
            if (!forwardOrRepeat (contactSequence, lastContactSequence_) ||
                !timeForwardOrRepeat (contact.observedAt, lastContact_.observedAt) ||
                (contact.observedAt == lastContact_.observedAt &&
                 !sameContact (contact, lastContact_)) ||
                (contactRepeat && !sameContact (contact, lastContact_)))
            {
                return StatusCode::InvalidArgument;
            }
        }
        if (hasDirectional_ && !directionalDomainChanged)
        {
            if (!forwardOrRepeat (directional.sequence, lastDirectional_.sequence) ||
                !timeForwardOrRepeat (directional.observedAt,
                                      lastDirectional_.observedAt) ||
                (directional.observedAt == lastDirectional_.observedAt &&
                 !sameDirectional (directional, lastDirectional_)) ||
                (directionalRepeat && !sameDirectional (directional, lastDirectional_)))
            {
                return StatusCode::InvalidArgument;
            }
        }

        const uint16_t ax         = absolute (directional.xPermille);
        const uint16_t ay         = absolute (directional.yPermille);
        const uint16_t magnitude  = ax > ay ? ax : ay;
        const bool     wasEngaged = hasDirectional_ && !directionalDomainChanged &&
                                    intent_.direction != InteractionDirection::Neutral;
        const bool     engaged =
            magnitude != 0 &&
            (wasEngaged ? magnitude >= config_.releaseMagnitudePermille
                        : magnitude >= config_.engageMagnitudePermille);
        const InteractionDirection direction =
            engaged ? directionFor (directional.xPermille, directional.yPermille)
                    : InteractionDirection::Neutral;
        const bool contactRecovery =
            hasContact_ && !contactRepeat && !contactDomainChanged &&
            (intent_.contactQuality == ContactQuality::SourceFault ||
             intent_.contactQuality == ContactQuality::TimingFault) &&
            contact.status.ok ();
        const bool recoveryBaseline =
            hasDirectional_ && !directionalRepeat &&
            (intent_.quality == InteractionQuality::SourceFault ||
             intent_.quality == InteractionQuality::TimingFault ||
             intent_.quality == InteractionQuality::Stale) &&
            contact.status.ok () && directional.status.ok ();

        candidate.owner             = this;
        candidate.generation        = generation_;
        candidate.now               = now;
        candidate.contactSource     = contactSource;
        candidate.contactSequence   = contactSequence;
        candidate.contact           = contact;
        candidate.directional       = directional;
        candidate.direction         = direction;
        candidate.magnitudePermille = magnitude;
        candidate.directionEvent    = !directionalRepeat && !directionalDomainChanged &&
                                      !recoveryBaseline && hasDirectional_ &&
                                      direction != intent_.direction;
        candidate.contactRepeat     = contactRepeat;
        candidate.directionalRepeat = directionalRepeat;
        candidate.contactDomainChanged     = contactDomainChanged;
        candidate.directionalDomainChanged = directionalDomainChanged;
        candidate.contactRecovery          = contactRecovery;
        candidate.recoveryBaseline         = recoveryBaseline;
        return StatusCode::Ok;
    }

    bool InteractionIntentPolicy::canCommit (
        const InteractionIntentPreview& candidate) const noexcept
    {
        return initialized_ && candidate.owner == this &&
               candidate.generation == generation_;
    }

    Status
    InteractionIntentPolicy::commit (const InteractionIntentPreview& candidate) noexcept
    {
        if (!canCommit (candidate))
        {
            return StatusCode::InvalidArgument;
        }

        if (candidate.contactDomainChanged || candidate.contactRecovery)
        {
            contact_.reset ();
        }

        const Status             contactResult = contact_.update   (candidate.contact);
        const ContactObservation contact       = contact_.snapshot ();
        const Duration           contactAge =
            candidate.now.elapsedSince (candidate.contact.observedAt);
        const Duration directionalAge =
            candidate.now.elapsedSince (candidate.directional.observedAt);
        const bool               stale = contactAge > config_.maximumContactAge ||
                                         directionalAge > config_.maximumDirectionalAge;
        const InteractionQuality quality =
            qualityFor (contact, candidate.directional.status, stale);
        const bool   usable = quality == InteractionQuality::Current ||
                              quality == InteractionQuality::StuckActive;
        const Status status =
            !contactResult.ok () ? contactResult : candidate.directional.status;

        intent_ = {candidate.contactSource,
                   candidate.directional.source,
                   candidate.now,
                   candidate.contactSequence,
                   candidate.directional.sequence,
                   usable ? candidate.direction : InteractionDirection::Neutral,
                   candidate.magnitudePermille,
                   usable && contact.qualifiedActive,
                   usable && !candidate.recoveryBaseline &&
                       !candidate.contactRepeat && contact.attackEvent &&
                       contact.disposition == ContactDisposition::Accepted,
                   usable && !candidate.recoveryBaseline &&
                       !candidate.contactRepeat && contact.releaseEvent,
                   usable && candidate.directionEvent,
                   quality,
                   contactAge,
                   directionalAge,
                   candidate.directional.saturated,
                   contact.quality,
                   contact.status,
                   candidate.directional.status,
                   status};

        lastContactSource_     = candidate.contactSource;
        lastDirectionalSource_ = candidate.directional.source;
        lastContact_           = candidate.contact;
        lastDirectional_       = candidate.directional;
        lastContactSequence_   = candidate.contactSequence;
        hasContact_            = true;
        hasDirectional_        = true;
        ++generation_;
        return intent_.status;
    }

    bool InteractionIntentPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    InteractionIntent InteractionIntentPolicy::snapshot () const noexcept
    {
        return intent_;
    }
    // clang-format on
} // namespace adk
