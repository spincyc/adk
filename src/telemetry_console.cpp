#include "telemetry_console.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint8_t  invalidSlot = TelemetryConsole::sourceCapacity;
        constexpr uint32_t maximumUnambiguousDuration =
            static_cast<uint32_t> (INT32_MAX);

        ConsoleSource emptySource () noexcept
        {
            return {0,
                    TelemetryKind::Temperature,
                    SampleQuality::SensorFault,
                    SequenceState::First,
                    Freshness::Stale,
                    0,
                    0,
                    PacketValidity::BadLength,
                    StatusCode::NotInitialized,
                    false};
        }

        bool kindValid (TelemetryKind kind) noexcept
        {
            return kind >= TelemetryKind::Temperature && kind <= TelemetryKind::Counter;
        }

        bool qualityValid (SampleQuality quality) noexcept
        {
            return quality >= SampleQuality::Valid &&
                   quality <= SampleQuality::StaleAtSource;
        }

        bool sequenceValid (SequenceState state) noexcept
        {
            return state >= SequenceState::First && state <= SequenceState::Reordered;
        }

        bool freshnessValid (Freshness freshness) noexcept
        {
            return freshness >= Freshness::Fresh && freshness <= Freshness::Stale;
        }

        bool packetValidityValid (PacketValidity validity) noexcept
        {
            return validity >= PacketValidity::Valid &&
                   validity <= PacketValidity::TrailingData;
        }

        uint8_t reasonPriority (ConsoleRecordReason reason) noexcept
        {
            return static_cast<uint8_t> (reason);
        }
    } // namespace

    ConsoleOutput::ConsoleOutput () noexcept
        : health (ConsoleHealth::Stopped), signal (ConsoleSignal::None),
          recordReason  (ConsoleRecordReason::None), selectedSourceId (0),
          status        (StatusCode::NotInitialized), recordStatus (StatusCode::Ok),
          recordAt      (0), selectedSlot (0), recordSourceSlot (invalidSlot),
          recordAttempt (0), writeRecord (false)
    {
    }

    TelemetryConsole::TelemetryConsole (const TelemetryConsoleConfig& config) noexcept
        : sourceConfigs_{}, sourceStates_{}, startupGrace_ (config.startupGrace),
          heartbeatPeriod_       (config.heartbeatPeriod), retryPeriod_ (config.retryPeriod),
          startedAt_             (0), lastEventAt_ (0), lastRecordAt_ (0), retryAt_ (0),
          output_                (),
          sourceCount_           (config.sourceCount),
          maximumRecordAttempts_ (config.maximumRecordAttempts),
          faultSignature_        (0), sourceConfigPresent_ (config.sources != nullptr),
          initialized_           (false), hasStartedAt_ (false),
          hasLastEvent_          (false),
          hasLastRecord_         (false), recordPending_ (false),
          attentionAcknowledged_ (false), startupComplete_ (false)
    {
        for (uint8_t slot = 0; slot < sourceCapacity; ++slot)
        {
            sourceStates_[slot].observation = emptySource ();
            sourceStates_[slot].known       = false;
        }

        if (config.sources != nullptr && config.sourceCount <= sourceCapacity)
        {
            for (uint8_t slot = 0; slot < config.sourceCount; ++slot)
            {
                sourceConfigs_[slot] = config.sources[slot];
            }
        }
    }

    TelemetryConsole::~TelemetryConsole () noexcept
    {
        shutdown ();
    }

    Status TelemetryConsole::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!configValid ())
        {
            clearRuntime (StatusCode::InvalidArgument);
            return output_.status;
        }

        clearRuntime (StatusCode::Ok);
        initialized_             = true;
        output_.health           = ConsoleHealth::Starting;
        output_.signal           = ConsoleSignal::None;
        output_.selectedSourceId = sourceConfigs_[0].sourceId;

        return StatusCode::Ok;
    }

    void TelemetryConsole::shutdown () noexcept
    {
        clearRuntime (StatusCode::NotInitialized);
    }

    bool TelemetryConsole::initialized () const noexcept
    {
        return initialized_;
    }

    Status TelemetryConsole::update (const ConsoleInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!inputValid (input))
        {
            setFault (StatusCode::InvalidArgument, input.observedAt, true);
            return output_.status;
        }

        if (hasStartedAt_ && hasLastEvent_ &&
            input.observedAt.elapsedSince (lastEventAt_).milliseconds () >
                maximumUnambiguousDuration)
        {
            setFault (StatusCode::InvalidArgument, input.observedAt, false);
            return output_.status;
        }

        if (input.nextPressEvent && input.acknowledgePressEvent)
        {
            setFault (StatusCode::InvalidArgument, input.observedAt, true);
            return output_.status;
        }

        for (uint8_t index = 0; index < input.sourceCount; ++index)
        {
            const ConsoleSource& source = input.sources[index];
            const uint8_t        slot   = findSlot (source.sourceId);

            if (slot == invalidSlot)
            {
                setFault (StatusCode::InvalidArgument, input.observedAt, true);
                return output_.status;
            }

            for (uint8_t prior = 0; prior < index; ++prior)
            {
                if (input.sources[prior].sourceId == source.sourceId)
                {
                    setFault (StatusCode::InvalidArgument,
                              input.observedAt,
                              true);
                    return output_.status;
                }
            }

            if (source.kind != sourceConfigs_[slot].kind || !sourceValid (source))
            {
                setFault (StatusCode::InvalidArgument, input.observedAt, true);
                return output_.status;
            }

        }

        if (!hasStartedAt_)
        {
            startedAt_    = input.observedAt;
            hasStartedAt_ = true;
        }

        lastEventAt_  = input.observedAt;
        hasLastEvent_ = true;

        bool    changed     = false;
        uint8_t changedSlot = invalidSlot;

        for (uint8_t slot = 0; slot < sourceCount_; ++slot)
        {
            const ConsoleSource* incoming = nullptr;

            for (uint8_t index = 0; index < input.sourceCount; ++index)
            {
                if (input.sources[index].sourceId == sourceConfigs_[slot].sourceId)
                {
                    incoming = &input.sources[index];
                    break;
                }
            }

            SourceState& state = sourceStates_[slot];

            if (incoming == nullptr)
            {
                if (state.known && state.observation.present)
                {
                    state.observation.present = false;
                    changed                   = true;
                    changedSlot               = slot;
                }

                continue;
            }

            const ConsoleSource& source = *incoming;

            if (!state.known || state.observation.quality != source.quality ||
                state.observation.sequenceState != source.sequenceState ||
                state.observation.freshness != source.freshness ||
                state.observation.value != source.value ||
                state.observation.decimalExponent != source.decimalExponent ||
                state.observation.packetValidity != source.packetValidity ||
                state.observation.observationStatus != source.observationStatus ||
                state.observation.present != source.present)
            {
                changed     = true;
                changedSlot = slot;
            }

            state.observation = source;
            state.known       = true;
        }

        if (input.nextPressEvent)
        {
            output_.selectedSlot =
                static_cast<uint8_t> ((output_.selectedSlot + 1U) % sourceCount_);
            output_.selectedSourceId = sourceConfigs_[output_.selectedSlot].sourceId;
        }

        const ConsoleHealth previousHealth = output_.health;

        output_.health = chooseHealth (input.observedAt);
        output_.status = StatusCode::Ok;

        if (output_.health == ConsoleHealth::Healthy ||
            output_.health == ConsoleHealth::Degraded)
        {
            startupComplete_ = true;
        }

        const uint32_t currentFaultSignature =
            output_.health == ConsoleHealth::Fault ? faultSignature () : 0;
        const bool newFault = output_.health == ConsoleHealth::Fault &&
                              currentFaultSignature != faultSignature_;

        if (newFault)
        {
            attentionAcknowledged_ = false;
        }

        const bool acknowledged =
            input.acknowledgePressEvent && output_.health == ConsoleHealth::Fault &&
            !newFault;

        if (acknowledged)
        {
            attentionAcknowledged_ = true;
        }

        if (output_.health == ConsoleHealth::Fault)
        {
            output_.signal = attentionAcknowledged_ ? ConsoleSignal::Notice
                                                    : ConsoleSignal::Attention;
        }
        else if (output_.health == ConsoleHealth::Degraded)
        {
            output_.signal         = ConsoleSignal::Notice;
            attentionAcknowledged_ = false;
        }
        else
        {
            output_.signal         = ConsoleSignal::None;
            attentionAcknowledged_ = false;
        }

        const ConsoleRecordReason reason = chooseRecordReason (
            previousHealth, changed, acknowledged, input.observedAt);

        requestRecord (reason, changedSlot, input.observedAt);
        faultSignature_ = currentFaultSignature;

        if (recordPending_ && recordDue (input.observedAt))
        {
            if (!output_.writeRecord)
            {
                ++output_.recordAttempt;
            }

            output_.writeRecord = true;
        }

        return output_.status;
    }

    Status TelemetryConsole::completeRecord (Status    status,
                                             TimePoint completedAt) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!recordPending_ || !output_.writeRecord)
        {
            return StatusCode::InvalidArgument;
        }

        if (hasLastEvent_ &&
            completedAt.elapsedSince (lastEventAt_).milliseconds () >
                maximumUnambiguousDuration)
        {
            return StatusCode::InvalidArgument;
        }

        lastEventAt_  = completedAt;
        hasLastEvent_ = true;

        output_.recordStatus = status;
        output_.writeRecord  = false;
        lastRecordAt_        = completedAt;
        hasLastRecord_       = true;

        if (status.ok ())
        {
            output_.recordReason     = ConsoleRecordReason::None;
            output_.recordSourceSlot = invalidSlot;
            output_.recordAttempt    = 0;
            recordPending_           = false;
            return status;
        }

        if (output_.recordAttempt >= maximumRecordAttempts_)
        {
            output_.recordReason     = ConsoleRecordReason::None;
            output_.recordSourceSlot = invalidSlot;
            recordPending_           = false;
            return status;
        }

        retryAt_ =
            TimePoint (completedAt.milliseconds () + retryPeriod_.milliseconds ());

        return status;
    }

    ConsoleOutput TelemetryConsole::output () const noexcept
    {
        return output_;
    }

    Result<ConsoleSource> TelemetryConsole::source (uint8_t slot) const noexcept
    {
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, emptySource ()};
        }

        if (slot >= sourceCount_)
        {
            return {StatusCode::InvalidArgument, emptySource ()};
        }

        return {StatusCode::Ok, sourceStates_[slot].observation};
    }

    bool TelemetryConsole::configValid () const noexcept
    {
        if (!sourceConfigPresent_ || sourceCount_ == 0 ||
            sourceCount_ > sourceCapacity || maximumRecordAttempts_ == 0 ||
            !durationValid (startupGrace_, true) ||
            !durationValid (heartbeatPeriod_, false) ||
            !durationValid (retryPeriod_, true))
        {
            return false;
        }

        for (uint8_t slot = 0; slot < sourceCount_; ++slot)
        {
            if (!kindValid (sourceConfigs_[slot].kind))
            {
                return false;
            }

            for (uint8_t prior = 0; prior < slot; ++prior)
            {
                if (sourceConfigs_[prior].sourceId == sourceConfigs_[slot].sourceId)
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool TelemetryConsole::inputValid (const ConsoleInput& input) const noexcept
    {
        return input.sourceCount <= sourceCapacity &&
               (input.sourceCount == 0 || input.sources != nullptr);
    }

    bool TelemetryConsole::sourceValid (const ConsoleSource& source) const noexcept
    {
        return kindValid (source.kind) && qualityValid (source.quality) &&
               sequenceValid       (source.sequenceState) &&
               freshnessValid      (source.freshness) &&
               packetValidityValid (source.packetValidity) &&
               source.decimalExponent >= -9 && source.decimalExponent <= 9;
    }

    uint8_t TelemetryConsole::findSlot (uint16_t sourceId) const noexcept
    {
        for (uint8_t slot = 0; slot < sourceCount_; ++slot)
        {
            if (sourceConfigs_[slot].sourceId == sourceId)
            {
                return slot;
            }
        }

        return invalidSlot;
    }

    uint32_t TelemetryConsole::faultSignature () const noexcept
    {
        uint32_t signature = 2166136261UL;

        for (uint8_t slot = 0; slot < sourceCount_; ++slot)
        {
            const SourceState& state = sourceStates_[slot];
            const ConsoleSource& source = state.observation;
            uint8_t fault = 0;

            if (!state.known || !source.present)
            {
                fault = 1;
            }
            else if (!source.observationStatus.ok ())
            {
                fault = static_cast<uint8_t> (
                    2U + static_cast<uint8_t> (source.observationStatus.error ()));
            }
            else if (source.packetValidity != PacketValidity::Valid)
            {
                fault = static_cast<uint8_t> (
                    16U + static_cast<uint8_t> (source.packetValidity));
            }
            else if (source.quality != SampleQuality::Valid)
            {
                fault = static_cast<uint8_t> (
                    32U + static_cast<uint8_t> (source.quality));
            }
            else if (source.freshness == Freshness::Stale)
            {
                fault = 48;
            }
            else if (source.sequenceState == SequenceState::Duplicate)
            {
                fault = 64;
            }
            else if (source.sequenceState == SequenceState::Reordered)
            {
                fault = 65;
            }

            signature ^= slot;
            signature *= 16777619UL;
            signature ^= fault;
            signature *= 16777619UL;
        }

        return signature;
    }

    ConsoleHealth TelemetryConsole::chooseHealth (TimePoint now) const noexcept
    {
        bool starting = false;
        bool degraded = false;

        for (uint8_t slot = 0; slot < sourceCount_; ++slot)
        {
            const SourceState& state = sourceStates_[slot];

            if (!state.known || !state.observation.present)
            {
                if (!startupComplete_ && !startupExpired (now))
                {
                    starting = true;
                    continue;
                }

                return ConsoleHealth::Fault;
            }

            const ConsoleSource& source = state.observation;

            if (!source.observationStatus.ok () ||
                source.packetValidity != PacketValidity::Valid ||
                source.quality != SampleQuality::Valid ||
                source.freshness == Freshness::Stale ||
                source.sequenceState == SequenceState::Duplicate ||
                source.sequenceState == SequenceState::Reordered)
            {
                return ConsoleHealth::Fault;
            }

            if (source.freshness == Freshness::Aging ||
                source.sequenceState == SequenceState::Gap)
            {
                degraded = true;
            }
        }

        if (starting)
        {
            return ConsoleHealth::Starting;
        }

        return degraded ? ConsoleHealth::Degraded : ConsoleHealth::Healthy;
    }

    ConsoleRecordReason
    TelemetryConsole::chooseRecordReason (ConsoleHealth previousHealth,
                                          bool observationChanged, bool acknowledged,
                                          TimePoint now) const noexcept
    {
        if (output_.health != previousHealth)
        {
            return ConsoleRecordReason::HealthTransition;
        }

        if (acknowledged)
        {
            return ConsoleRecordReason::Acknowledgement;
        }

        if (observationChanged)
        {
            return ConsoleRecordReason::Observation;
        }

        if (!hasLastRecord_ || now.elapsedSince (lastRecordAt_) >= heartbeatPeriod_)
        {
            return ConsoleRecordReason::Heartbeat;
        }

        return ConsoleRecordReason::None;
    }

    void TelemetryConsole::requestRecord (ConsoleRecordReason reason,
                                          uint8_t sourceSlot, TimePoint now) noexcept
    {
        if (reason == ConsoleRecordReason::None)
        {
            return;
        }

        if (!recordPending_)
        {
            recordPending_           = true;
            output_.recordReason     = reason;
            output_.recordSourceSlot = sourceSlot;
            output_.recordAttempt    = 1;
            output_.writeRecord      = true;
            output_.recordAt         = now;
            retryAt_                 = now;
            return;
        }

        if (reasonPriority (reason) > reasonPriority (output_.recordReason))
        {
            output_.recordReason = reason;
            output_.recordAt     = now;

            if (sourceSlot != invalidSlot)
            {
                output_.recordSourceSlot = sourceSlot;
            }
        }
    }

    void TelemetryConsole::clearRuntime (Status status) noexcept
    {
        for (uint8_t slot = 0; slot < sourceCapacity; ++slot)
        {
            sourceStates_[slot].observation = emptySource ();
            sourceStates_[slot].known       = false;

            if (slot < sourceCount_)
            {
                sourceStates_[slot].observation.sourceId =
                    sourceConfigs_[slot].sourceId;
                sourceStates_[slot].observation.kind =
                    sourceConfigs_[slot].kind;
            }
        }

        startedAt_               = TimePoint (0);
        lastEventAt_             = TimePoint (0);
        lastRecordAt_            = TimePoint (0);
        retryAt_                 = TimePoint (0);
        output_.health           = ConsoleHealth::Stopped;
        output_.signal           = ConsoleSignal::None;
        output_.recordReason     = ConsoleRecordReason::None;
        output_.selectedSourceId = sourceCount_ == 0 ? 0 : sourceConfigs_[0].sourceId;
        output_.status           = status;
        output_.recordStatus     = StatusCode::Ok;
        output_.recordAt         = TimePoint (0);
        output_.selectedSlot     = 0;
        output_.recordSourceSlot = invalidSlot;
        output_.recordAttempt    = 0;
        output_.writeRecord      = false;
        faultSignature_          = 0;
        initialized_             = false;
        hasStartedAt_            = false;
        hasLastEvent_            = false;
        hasLastRecord_           = false;
        recordPending_           = false;
        attentionAcknowledged_   = false;
        startupComplete_         = false;
    }

    void TelemetryConsole::setFault (Status    status,
                                     TimePoint now,
                                     bool      recordFault) noexcept
    {
        const ConsoleHealth previousHealth = output_.health;

        const uint32_t signature =
            0x80000000UL | static_cast<uint8_t> (status.error ());
        const bool newFault = signature != faultSignature_;

        output_.health         = ConsoleHealth::Fault;
        output_.signal         = newFault || !attentionAcknowledged_
                                     ? ConsoleSignal::Attention
                                     : ConsoleSignal::Notice;
        output_.status         = status;
        attentionAcknowledged_ = newFault ? false : attentionAcknowledged_;
        faultSignature_        = signature;

        const ConsoleRecordReason reason = previousHealth == ConsoleHealth::Fault
                                               ? ConsoleRecordReason::Observation
                                               : ConsoleRecordReason::HealthTransition;

        if (recordFault)
        {
            requestRecord (reason, invalidSlot, now);
        }
    }

    bool TelemetryConsole::recordDue (TimePoint now) const noexcept
    {
        return output_.writeRecord || now.elapsedSince (retryAt_).milliseconds () <=
                                          maximumUnambiguousDuration;
    }

    bool TelemetryConsole::startupExpired (TimePoint now) const noexcept
    {
        return now.elapsedSince (startedAt_) >= startupGrace_;
    }

    bool TelemetryConsole::durationValid (Duration duration,
                                          bool     allowZero) const noexcept
    {
        return duration.milliseconds () <= maximumUnambiguousDuration &&
               (allowZero || duration.milliseconds () != 0);
    }
} // namespace adk
