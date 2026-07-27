#include "inert_channel_assessor.h"

namespace adk {

    namespace {

        constexpr uint32_t maximumUnambiguousAge = 0x7fffffffu;

        bool validObservation (InertObservation observation) noexcept
        {
            switch (observation)
            {
                case InertObservation::Open:
                case InertObservation::Closed:
                case InertObservation::ShortSimulated:
                case InertObservation::Unavailable: return true;
            }

            return false;
        }

        InertChannelAssessment unavailableAssessment (InertChannelId channel) noexcept
        {
            return {channel, InertChannelState::Unavailable, TimePoint ()};
        }

        InertChannelState classify (InertObservation primary,
                                    InertObservation redundant) noexcept
        {
            if (primary == InertObservation::Unavailable ||
                redundant == InertObservation::Unavailable)
            {
                return InertChannelState::Unavailable;
            }

            if (primary != redundant)
            {
                return InertChannelState::Contradictory;
            }

            switch (primary)
            {
                case InertObservation::Open: return InertChannelState::Open;
                case InertObservation::Closed: return InertChannelState::Closed;
                case InertObservation::ShortSimulated:
                    return InertChannelState::ShortSimulated;
                case InertObservation::Unavailable:
                    return InertChannelState::Unavailable;
            }

            return InertChannelState::Unavailable;
        }
    } // namespace

    RecordedInertObservationSource::RecordedInertObservationSource (
        const RecordedInertObservationSet* sets,
        uint8_t                            setCount) noexcept
        : sets_        (sets)
        , setCount_    (setCount)
        , nextSet_     (0)
        , selectedSet_ (0)
        , selected_    (false)
        , initialized_ (false)
    {
    }

    RecordedInertObservationSource::~RecordedInertObservationSource () noexcept
    {
        shutdown ();
    }

    Status RecordedInertObservationSource::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validSets ())
        {
            return StatusCode::InvalidArgument;
        }

        nextSet_     = 0;
        selectedSet_ = 0;
        selected_    = false;
        initialized_ = true;
        return StatusCode::Ok;
    }

    void RecordedInertObservationSource::shutdown () noexcept
    {
        nextSet_     = 0;
        selectedSet_ = 0;
        selected_    = false;
        initialized_ = false;
    }

    bool RecordedInertObservationSource::initialized () const noexcept
    {
        return initialized_;
    }

    Status RecordedInertObservationSource::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        while (nextSet_ < setCount_ &&
               now.elapsedSince (sets_[nextSet_].dueAt).milliseconds () <=
                   maximumUnambiguousAge)
        {
            selectedSet_ = nextSet_;
            selected_    = true;
            ++nextSet_;
        }

        return StatusCode::Ok;
    }

    RecordedInertObservationSnapshot
    RecordedInertObservationSource::snapshot () const noexcept
    {
        if (!initialized_ || !selected_)
        {
            return {nullptr, 0, TimePoint (), false};
        }

        return {sets_[selectedSet_].observations,
                InertChannelAssessor::capacity,
                sets_[selectedSet_].dueAt,
                true};
    }

    bool RecordedInertObservationSource::validSets () const noexcept
    {
        if (sets_ == nullptr || setCount_ == 0 || setCount_ > capacity)
        {
            return false;
        }

        for (uint8_t setIndex = 0; setIndex < setCount_; ++setIndex)
        {
            bool channels[InertChannelAssessor::capacity] = {};

            for (uint8_t observationIndex = 0;
                 observationIndex < InertChannelAssessor::capacity;
                 ++observationIndex)
            {
                const InertChannelId channel =
                    sets_[setIndex].observations[observationIndex].channel;
                const InertChannelObservation& observation =
                    sets_[setIndex].observations[observationIndex];
                const bool future =
                    sets_[setIndex].dueAt
                        .elapsedSince (observation.observedAt)
                        .milliseconds () > maximumUnambiguousAge;

                if (channel >= InertChannelAssessor::capacity || channels[channel] ||
                    !validObservation (observation.primary) ||
                    !validObservation (observation.redundant) || future)
                {
                    return false;
                }

                channels[channel] = true;
            }

            if (setIndex > 0)
            {
                const uint32_t step =
                    sets_[setIndex].dueAt
                        .elapsedSince (sets_[setIndex - 1].dueAt)
                        .milliseconds ();

                if (step == 0 || step > maximumUnambiguousAge)
                {
                    return false;
                }
            }
        }

        return true;
    }

    InertChannelAssessor::InertChannelAssessor (Duration staleAfter) noexcept
        : staleAfter_  (staleAfter)
        , initialized_ (false)
    {
        for (uint8_t channel = 0; channel < capacity; ++channel)
        {
            slots_[channel].observation = {
                channel, InertObservation::Unavailable,
                InertObservation::Unavailable, TimePoint ()};
            slots_[channel].present = false;
        }
    }

    InertChannelAssessor::~InertChannelAssessor () noexcept
    {
        shutdown ();
    }

    Status InertChannelAssessor::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (staleAfter_.milliseconds () == 0 ||
            staleAfter_.milliseconds () > maximumUnambiguousAge)
        {
            return StatusCode::InvalidArgument;
        }

        for (uint8_t channel = 0; channel < capacity; ++channel)
        {
            slots_[channel].present = false;
        }

        initialized_ = true;
        return StatusCode::Ok;
    }

    void InertChannelAssessor::shutdown () noexcept
    {
        for (uint8_t channel = 0; channel < capacity; ++channel)
        {
            slots_[channel].present = false;
        }

        initialized_ = false;
    }

    bool InertChannelAssessor::initialized () const noexcept
    {
        return initialized_;
    }

    Status InertChannelAssessor::update (TimePoint                      now,
                                         const InertChannelObservation* observations,
                                         uint8_t observationCount) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (observationCount > capacity ||
            (observationCount > 0 && observations == nullptr))
        {
            return StatusCode::InvalidArgument;
        }

        bool seen[capacity] = {};

        for (uint8_t index = 0; index < observationCount; ++index)
        {
            const InertChannelObservation& observation = observations[index];
            const bool future =
                now.elapsedSince (observation.observedAt).milliseconds () >
                maximumUnambiguousAge;

            if (observation.channel >= capacity || seen[observation.channel] ||
                !validObservation (observation.primary) ||
                !validObservation (observation.redundant) ||
                future)
            {
                return StatusCode::InvalidArgument;
            }

            seen[observation.channel] = true;
        }

        for (uint8_t index = 0; index < observationCount; ++index)
        {
            const InertChannelObservation& observation = observations[index];

            slots_[observation.channel].observation = observation;
            slots_[observation.channel].present     = true;
        }

        return StatusCode::Ok;
    }

    Result<InertChannelAssessment>
    InertChannelAssessor::assessment (InertChannelId channel,
                                      TimePoint      now) const noexcept
    {
        const InertChannelAssessment unavailable = unavailableAssessment (channel);

        if (!initialized_)
        {
            return Result<InertChannelAssessment> (StatusCode::NotInitialized,
                                                   unavailable);
        }

        if (channel >= capacity)
        {
            return Result<InertChannelAssessment> (StatusCode::InvalidArgument,
                                                   unavailable);
        }

        const Slot& slot = slots_[channel];

        if (!slot.present)
        {
            return Result<InertChannelAssessment> (StatusCode::Ok, unavailable);
        }

        const uint32_t age =
            now.elapsedSince (slot.observation.observedAt).milliseconds ();

        InertChannelState state =
            classify (slot.observation.primary, slot.observation.redundant);

        if (age > maximumUnambiguousAge)
        {
            return Result<InertChannelAssessment> (StatusCode::InvalidArgument,
                                                   unavailable);
        }

        if (age > staleAfter_.milliseconds ())
        {
            state = InertChannelState::Stale;
        }

        const InertChannelAssessment result = {channel, state,
                                               slot.observation.observedAt};
        return Result<InertChannelAssessment> (StatusCode::Ok, result);
    }

} // namespace adk
