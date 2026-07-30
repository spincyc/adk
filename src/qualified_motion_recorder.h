#pragma once

#include "inertial_record_qualification.h"
#include "orientation_presentation.h"

#include <stdint.h>

namespace adk {

    enum struct MotionRecorderMode : uint8_t
    {
        Inert,
        AwaitingQualification,
        Ready,
        Recording,
        Complete,
        Fault,
        Shutdown
    };

    enum struct MotionScriptStep : uint8_t
    {
        Rest,
        TiltForward,
        TiltBack,
        TiltLeft,
        TiltRight,
        ReturnToRest
    };

    enum struct MotionRecorderHealth : uint8_t
    {
        Unknown,
        Ready,
        Recording,
        Complete,
        SourceFault,
        Stale,
        Saturated,
        CapacityExhausted
    };

    enum struct MotionDisplayToken : uint8_t
    {
        QualifySource,
        ReadyToRecord,
        HoldRest,
        TiltForward,
        TiltBack,
        TiltLeft,
        TiltRight,
        ReturnToRest,
        RecordingComplete,
        Fault
    };

    struct MotionRecorderConfig
    {
        uint16_t         recordSchemaRevision;
        uint16_t         normalizationRevision;
        uint16_t         qualificationRevision;
        uint16_t         recorderRevision;
        uint16_t         maximumRecordCount;
        uint32_t         traceToken;
        Duration         maximumRecordAge;
        Duration         minimumStepDuration;
        InertialSource   expectedSource;
        OrientationConfig orientation;
    };

    enum struct MotionRecorderCommand : uint8_t
    {
        None,
        Advance,
        Reset,
        RequestExport,
        AcknowledgeExport
    };

    struct MotionRecorderControl
    {
        uint8_t   sourceId;
        uint32_t  sequence;
        TimePoint observedAt;
        uint16_t  qualificationRevision;
        uint32_t  qualificationLifecycleGeneration;
        uint32_t  qualificationAttemptId;
        uint32_t  qualificationDigest;
        uint32_t  traceToken;
        MotionRecorderCommand command;
        Status    status;
    };

    struct MotionPresentationIntent
    {
        MotionDisplayToken    token;
        MotionRecorderHealth  health;
        uint8_t               rgbRed;
        uint8_t               rgbGreen;
        uint8_t               rgbBlue;
        bool                  orientationValid;
        int16_t               pitchTenthsDegree;
        int16_t               rollTenthsDegree;
    };

    struct MotionRecordImage
    {
        static constexpr uint16_t capacity = 128;

        uint8_t bytes[capacity];
    };

    enum struct MotionRecordValidity : uint8_t
    {
        Valid,
        BadLength,
        BadFraming,
        BadIntegrity,
        BadSemanticValue
    };

    struct DecodedMotionRecord
    {
        uint16_t             recorderRevision;
        uint32_t             lifecycleGeneration;
        uint32_t             sessionId;
        uint16_t             ordinal;
        MotionScriptStep     scriptStep;
        MotionRecorderHealth health;
        uint16_t             qualificationRevision;
        uint32_t             qualificationLifecycleGeneration;
        uint32_t             qualificationAttemptId;
        uint32_t             qualificationDigest;
        uint32_t             recordDigest;
        uint32_t             traceToken;
        SourceAxisMapping    sourceToQualificationFrame;
        InertialRecord       mappedRecord;
        OrientationEstimate orientation;
    };

    struct MotionRecordCodec
    {
        static constexpr uint8_t version = 1;

        MotionRecordValidity decode (
            const MotionRecordImage& image,
            DecodedMotionRecord&     output) const noexcept;
    };

    uint32_t motionQualificationDigest (
        const InertialQualificationEvidence& evidence) noexcept;
    uint32_t motionRecordDigest (const InertialRecord& record) noexcept;

    struct MotionRecorderResult
    {
        uint32_t                      sessionId;
        uint32_t                      lifecycleGeneration;
        MotionRecorderMode            mode;
        MotionRecorderHealth          health;
        MotionScriptStep              scriptStep;
        uint16_t                      recordCount;
        uint16_t                      recordCapacity;
        InertialRecord                latestRecord;
        InertialQualificationEvidence qualification;
        MotionPresentationIntent      presentation;
        bool                          exportRequested;
        Status                        status;
    };

    struct QualifiedMotionRecorder
    {
        explicit QualifiedMotionRecorder (
            const MotionRecorderConfig& config) noexcept;

        QualifiedMotionRecorder (const QualifiedMotionRecorder&) = delete;
        QualifiedMotionRecorder&
        operator= (const QualifiedMotionRecorder&) = delete;
        QualifiedMotionRecorder (QualifiedMotionRecorder&&) = delete;
        QualifiedMotionRecorder&
        operator= (QualifiedMotionRecorder&&) = delete;

        Status initialize (TimePoint now, uint16_t recordCapacity) noexcept;

        Status qualify (
            TimePoint                            now,
            const InertialQualificationEvidence& evidence) noexcept;

        Status begin (TimePoint now, uint32_t sessionId) noexcept;

        Status update (TimePoint                    now,
                       const InertialRecord&         record,
                       const MotionRecorderControl& control,
                       MotionRecordImage*           records,
                       uint16_t                     recordCapacity) noexcept;

        Status acknowledgeExport (TimePoint now) noexcept;

        Status reset    (TimePoint now) noexcept;
        Status shutdown (TimePoint now) noexcept;
        Status result   (MotionRecorderResult& output) const noexcept;

        bool initialized () const noexcept;

      private:
        Status resetState (TimePoint now) noexcept;

        MotionRecorderConfig            config_;
        uint16_t                        recordCapacity_;
        MotionRecorderResult            result_;
        OrientationPolicy               orientation_;
        TimePoint                       stepStartedAt_;
        uint32_t                        lastSessionId_;
        uint32_t                        lastRecordSequence_;
        TimePoint                       lastRecordObservedAt_;
        MotionRecorderControl           lastAcceptedControl_;
        uint32_t                        qualificationDigest_;
        uint32_t                        lifecycleGeneration_;
        bool                            initialized_;
        bool                            shutdown_;
        bool                            hasRecord_;
    };
} // namespace adk
