#pragma once

#include "record_sink.h"
#include "telemetry_console.h"
#include "telemetry_record.h"

namespace adk {

    struct TelemetryConsoleOperator
    {
        virtual ~TelemetryConsoleOperator () noexcept;

        virtual Status initialize            () noexcept             = 0;
        virtual void   shutdown              () noexcept             = 0;
        virtual bool   initialized           () const noexcept       = 0;
        virtual Status update                (TimePoint now) noexcept = 0;
        virtual bool   nextPressEvent        () const noexcept       = 0;
        virtual bool   acknowledgePressEvent () const noexcept       = 0;
    };

    struct TelemetryConsolePresentation
    {
        virtual ~TelemetryConsolePresentation () noexcept;

        virtual Status initialize  () noexcept = 0;
        virtual void   shutdown    () noexcept = 0;
        virtual bool   initialized () const noexcept = 0;
        virtual Status present     (TimePoint            now,
                                    const ConsoleOutput& output,
                                    const ConsoleSource& selected) noexcept = 0;
    };

    struct TelemetryConsoleProjectSnapshot
    {
        ConsoleOutput console;
        Status        operatorStatus;
        Status        presentationStatus;
        Status        recordStatus;
        bool          initialized;
    };

    struct TelemetryConsoleProject
    {
        TelemetryConsoleProject  (TelemetryConsole&             console,
                                  TelemetryConsoleOperator&     operatorInput,
                                  TelemetryConsolePresentation& presentation,
                                  RecordSink&                   records) noexcept;
        ~TelemetryConsoleProject () noexcept;

        TelemetryConsoleProject (
            const TelemetryConsoleProject&) = delete;
        TelemetryConsoleProject& operator= (
            const TelemetryConsoleProject&) = delete;
        TelemetryConsoleProject (TelemetryConsoleProject&&) = delete;
        TelemetryConsoleProject& operator= (
            TelemetryConsoleProject&&) = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;
        Status update      (TimePoint            now,
                           const ConsoleSource* sources,
                           uint8_t              sourceCount) noexcept;

        TelemetryConsoleProjectSnapshot snapshot () const noexcept;

      private:
        Status present  (TimePoint now) noexcept;
        Status record   (TimePoint now) noexcept;
        void   rollback () noexcept;
        void   clear    () noexcept;

        TelemetryConsole*             console_;
        TelemetryConsoleOperator*     operatorInput_;
        TelemetryConsolePresentation* presentation_;
        RecordSink*                   records_;
        TelemetryRecordEncoder        encoder_;
        StableRecord                  pendingRecord_;
        TelemetryConsoleProjectSnapshot snapshot_;
        ConsoleRecordReason           pendingReason_;
        uint8_t                       pendingSourceSlot_;
        bool                          consoleAcquired_;
        bool                          operatorAcquired_;
        bool                          presentationAcquired_;
        bool                          recordsAcquired_;
        bool                          pendingRecordReady_;
        bool                          initialized_;
    };
} // namespace adk
