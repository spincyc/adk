#pragma once

#include "observation_tracker.h"
#include "status.h"
#include "telemetry_packet.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct ConsoleHealth : uint8_t
    {
        Starting,
        Healthy,
        Degraded,
        Fault,
        Stopped
    };

    enum struct ConsoleSignal : uint8_t
    {
        None,
        Notice,
        Attention
    };

    enum struct ConsoleRecordReason : uint8_t
    {
        None,
        Heartbeat,
        Observation,
        Acknowledgement,
        HealthTransition
    };

    struct ConsoleSourceConfig
    {
        uint16_t      sourceId;
        TelemetryKind kind;
    };

    struct TelemetryConsoleConfig
    {
        const ConsoleSourceConfig* sources;
        uint8_t                    sourceCount;
        Duration                   startupGrace;
        Duration                   heartbeatPeriod;
        Duration                   retryPeriod;
        uint8_t                    maximumRecordAttempts;
    };

    struct ConsoleSource
    {
        uint16_t       sourceId;
        TelemetryKind  kind;
        SampleQuality  quality;
        SequenceState  sequenceState;
        Freshness      freshness;
        int32_t        value;
        int8_t         decimalExponent;
        PacketValidity packetValidity;
        Status         observationStatus;
        bool           present;
    };

    struct ConsoleInput
    {
        const ConsoleSource* sources;
        uint8_t              sourceCount;
        bool                 nextPressEvent;
        bool                 acknowledgePressEvent;
        TimePoint            observedAt;
    };

    struct ConsoleOutput
    {
        ConsoleOutput () noexcept;

        ConsoleHealth       health;
        ConsoleSignal       signal;
        ConsoleRecordReason recordReason;
        uint16_t            selectedSourceId;
        Status              status;
        Status              recordStatus;
        TimePoint           recordAt;
        uint8_t             selectedSlot;
        uint8_t             recordSourceSlot;
        uint8_t             recordAttempt;
        bool                writeRecord;
    };

    struct TelemetryConsole
    {
        static constexpr uint8_t sourceCapacity = 8;

        explicit TelemetryConsole (const TelemetryConsoleConfig& config) noexcept;
        ~TelemetryConsole         () noexcept;

        TelemetryConsole (const TelemetryConsole&)            = delete;
        TelemetryConsole& operator= (const TelemetryConsole&) = delete;
        TelemetryConsole (TelemetryConsole&&)                 = delete;
        TelemetryConsole& operator= (TelemetryConsole&&)      = delete;

        Status initialize     () noexcept;
        void   shutdown       () noexcept;
        bool   initialized    () const noexcept;
        Status update         (const ConsoleInput& input) noexcept;
        Status completeRecord (Status status, TimePoint completedAt) noexcept;

        ConsoleOutput output         () const noexcept;
        Result<ConsoleSource> source (uint8_t slot) const noexcept;

      private:
        struct SourceState
        {
            ConsoleSource observation;
            bool          known;
        };

        bool                configValid        () const noexcept;
        bool                inputValid         (const ConsoleInput& input) const noexcept;
        bool                sourceValid        (const ConsoleSource& source) const noexcept;
        uint8_t             findSlot           (uint16_t sourceId) const noexcept;
        uint32_t            faultSignature     () const noexcept;
        ConsoleHealth       chooseHealth       (TimePoint now) const noexcept;
        ConsoleRecordReason chooseRecordReason (ConsoleHealth previousHealth,
                                                bool          observationChanged,
                                                bool          acknowledged,
                                                TimePoint     now) const noexcept;
        void requestRecord (ConsoleRecordReason reason, uint8_t sourceSlot,
                            TimePoint now) noexcept;
        void clearRuntime   (Status status) noexcept;
        void setFault       (Status status, TimePoint now,
                             bool recordFault) noexcept;
        bool recordDue      (TimePoint now) const noexcept;
        bool startupExpired (TimePoint now) const noexcept;
        bool durationValid  (Duration duration, bool allowZero) const noexcept;

        ConsoleSourceConfig sourceConfigs_[sourceCapacity];
        SourceState         sourceStates_[sourceCapacity];
        Duration            startupGrace_;
        Duration            heartbeatPeriod_;
        Duration            retryPeriod_;
        TimePoint           startedAt_;
        TimePoint           lastEventAt_;
        TimePoint           lastRecordAt_;
        TimePoint           retryAt_;
        ConsoleOutput       output_;
        uint8_t             sourceCount_;
        uint8_t             maximumRecordAttempts_;
        uint32_t            faultSignature_;
        bool                sourceConfigPresent_;
        bool                initialized_;
        bool                hasStartedAt_;
        bool                hasLastEvent_;
        bool                hasLastRecord_;
        bool                recordPending_;
        bool                attentionAcknowledged_;
        bool                startupComplete_;
    };
} // namespace adk
