#include "observation_tracker.h"

namespace adk {

    namespace {

        bool validKind (TelemetryKind kind) noexcept
        {
            switch (kind)
            {
                case TelemetryKind::Temperature: return true;
                case TelemetryKind::RelativeHumidity: return true;
                case TelemetryKind::Distance: return true;
                case TelemetryKind::Contact: return true;
                case TelemetryKind::Counter: return true;
            }

            return false;
        }

        bool validQuality (SampleQuality quality) noexcept
        {
            switch (quality)
            {
                case SampleQuality::Valid: return true;
                case SampleQuality::SensorFault: return true;
                case SampleQuality::OutOfRange: return true;
                case SampleQuality::StaleAtSource: return true;
            }

            return false;
        }

        TelemetrySample emptySample (uint16_t sourceId) noexcept
        {
            return {
                sourceId, 0, 0, TelemetryKind::Temperature, SampleQuality::SensorFault,
                0,        0};
        }
    } // namespace

    ObservationTracker::ObservationTracker (
        uint16_t sourceId, const ObservationTrackerConfig& config) noexcept
        : sourceId_ (sourceId), config_ (config),
          state_ ({emptySample (sourceId), SequenceState::First, Freshness::Stale,
                   Duration (), StatusCode::NotInitialized}),
          receivedAt_ (), initialized_ (false), hasSample_ (false)
    {
    }

    ObservationTracker::~ObservationTracker () noexcept
    {
        shutdown ();
    }

    Status ObservationTracker::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Duration maximumComparableAge (0x7fffffff);

        if (config_.agingAfter >= config_.staleAfter ||
            config_.staleAfter == Duration () ||
            config_.agingAfter > maximumComparableAge ||
            config_.staleAfter > maximumComparableAge)
        {
            return StatusCode::InvalidArgument;
        }

        state_       = {emptySample (sourceId_), SequenceState::First, Freshness::Stale,
                        Duration (), StatusCode::Ok};
        receivedAt_  = TimePoint ();
        hasSample_   = false;
        initialized_ = true;
        return StatusCode::Ok;
    }

    void ObservationTracker::shutdown () noexcept
    {
        initialized_ = false;
        hasSample_   = false;
        state_       = {emptySample (sourceId_), SequenceState::First, Freshness::Stale,
                        Duration (), StatusCode::NotInitialized};
    }

    bool ObservationTracker::initialized () const noexcept
    {
        return initialized_;
    }

    Status ObservationTracker::accept (const TelemetrySample& sample,
                                       TimePoint              receivedAt) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (sample.sourceId != sourceId_ || !validKind (sample.kind) ||
            !validQuality (sample.quality))
        {
            return StatusCode::InvalidArgument;
        }

        if (!hasSample_)
        {
            state_.sample        = sample;
            state_.sequenceState = SequenceState::First;
            state_.freshness     = Freshness::Fresh;
            state_.age           = Duration ();
            state_.status        = StatusCode::Ok;
            receivedAt_          = receivedAt;
            hasSample_           = true;
            return StatusCode::Ok;
        }

        const uint16_t difference =
            static_cast<uint16_t> (sample.sequence - state_.sample.sequence);

        if (difference == 0)
        {
            state_.sequenceState = SequenceState::Duplicate;
            return StatusCode::Ok;
        }

        // The exact half range is ambiguous and cannot replace accepted evidence.
        if (difference >= 0x8000)
        {
            state_.sequenceState = SequenceState::Reordered;
            return StatusCode::Ok;
        }

        state_.sample = sample;
        state_.sequenceState =
            difference == 1 ? SequenceState::InOrder : SequenceState::Gap;
        state_.freshness = Freshness::Fresh;
        state_.age       = Duration ();
        state_.status    = StatusCode::Ok;
        receivedAt_      = receivedAt;
        return StatusCode::Ok;
    }

    Status ObservationTracker::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!hasSample_)
        {
            return StatusCode::Ok;
        }

        state_.age = now.elapsedSince (receivedAt_);

        if (state_.age >= config_.staleAfter)
        {
            state_.freshness = Freshness::Stale;
        }
        else if (state_.age >= config_.agingAfter)
        {
            state_.freshness = Freshness::Aging;
        }
        else
        {
            state_.freshness = Freshness::Fresh;
        }

        return StatusCode::Ok;
    }

    ObservationState ObservationTracker::state () const noexcept
    {
        return state_;
    }
} // namespace adk
