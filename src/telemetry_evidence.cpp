#include "telemetry_evidence.h"

namespace adk {

    TelemetryEvidenceSignal
    TelemetryEvidenceModel::decide (PacketValidity          validity,
                                    const ObservationState& observation,
                                    Status                  status) const noexcept
    {
        if (!status.ok ())
        {
            return TelemetryEvidenceSignal::Fault;
        }

        if (validity != PacketValidity::Valid)
        {
            return TelemetryEvidenceSignal::Corrupt;
        }

        if (observation.freshness == Freshness::Stale)
        {
            return TelemetryEvidenceSignal::Stale;
        }

        if (observation.freshness == Freshness::Aging ||
            observation.sequenceState == SequenceState::Duplicate ||
            observation.sequenceState == SequenceState::Gap ||
            observation.sequenceState == SequenceState::Reordered)
        {
            return TelemetryEvidenceSignal::GapOrAging;
        }

        return TelemetryEvidenceSignal::Fresh;
    }

    TelemetryFixtureSchedule::TelemetryFixtureSchedule () noexcept
        : startedAt_ (), cycle_ (0), nextSequence_ (1), phase_ (0),
          initialized_ (false), started_ (false)
    {
    }

    TelemetryFixtureSchedule::~TelemetryFixtureSchedule () noexcept
    {
        shutdown ();
    }

    Status TelemetryFixtureSchedule::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        startedAt_    = TimePoint ();
        cycle_        = 0;
        nextSequence_ = 1;
        phase_        = 0;
        initialized_  = true;
        started_      = false;
        return StatusCode::Ok;
    }

    void TelemetryFixtureSchedule::shutdown () noexcept
    {
        startedAt_    = TimePoint ();
        cycle_        = 0;
        nextSequence_ = 1;
        phase_        = 0;
        initialized_  = false;
        started_      = false;
    }

    bool TelemetryFixtureSchedule::initialized () const noexcept
    {
        return initialized_;
    }

    Result<TelemetryFixtureDecision>
    TelemetryFixtureSchedule::update (TimePoint now) noexcept
    {
        const TelemetryFixtureDecision none = {TelemetryFixtureAction::None,
                                               nextSequence_};

        if (!initialized_)
        {
            return {StatusCode::NotInitialized, none};
        }

        if (!started_)
        {
            startedAt_ = now;
            cycle_     = 0;
            phase_     = 0;
            started_   = true;

            const TelemetryFixtureDecision first = {TelemetryFixtureAction::Accept,
                                                    nextSequence_};
            ++nextSequence_;
            return {StatusCode::Ok, first};
        }

        const uint32_t totalElapsed = now.elapsedSince (startedAt_).milliseconds ();
        const uint32_t cycle        = totalElapsed / 12000;
        const uint32_t elapsed      = totalElapsed % 12000;
        const uint8_t  phase        = elapsed < 1000   ? 0
                                      : elapsed < 3000 ? 1
                                      : elapsed < 5000 ? 2
                                                       : 3;

        if (phase == phase_ && cycle == cycle_)
        {
            return {StatusCode::Ok, none};
        }

        cycle_ = cycle;
        phase_ = phase;

        if (phase == 0)
        {
            const TelemetryFixtureDecision recovery = {TelemetryFixtureAction::Accept,
                                                       nextSequence_};
            ++nextSequence_;
            return {StatusCode::Ok, recovery};
        }

        if (phase == 1)
        {
            ++nextSequence_;
            const TelemetryFixtureDecision gap = {TelemetryFixtureAction::Accept,
                                                  nextSequence_};
            ++nextSequence_;
            return {StatusCode::Ok, gap};
        }

        if (phase == 2)
        {
            return {StatusCode::Ok, {TelemetryFixtureAction::Corrupt, nextSequence_}};
        }

        return {StatusCode::Ok, {TelemetryFixtureAction::Silence, nextSequence_}};
    }
} // namespace adk
