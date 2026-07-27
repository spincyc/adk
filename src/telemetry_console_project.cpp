#include "telemetry_console_project.h"

namespace adk {

    TelemetryConsoleOperator::~TelemetryConsoleOperator () noexcept
    {
    }

    TelemetryConsolePresentation::~TelemetryConsolePresentation () noexcept
    {
    }

    TelemetryConsoleProject::TelemetryConsoleProject (
        TelemetryConsole& console, TelemetryConsoleOperator& operatorInput,
        TelemetryConsolePresentation& presentation, RecordSink& records) noexcept
        : console_ (&console), operatorInput_ (&operatorInput),
          presentation_              (&presentation), records_ (&records), encoder_ (),
          pendingRecord_ {}, snapshot_ {},
          pendingReason_        (ConsoleRecordReason::None),
          pendingSourceSlot_    (TelemetryConsole::sourceCapacity),
          consoleAcquired_      (false), operatorAcquired_ (false),
          presentationAcquired_ (false), recordsAcquired_ (false),
          pendingRecordReady_   (false), initialized_ (false)
    {
        clear ();
    }

    TelemetryConsoleProject::~TelemetryConsoleProject () noexcept
    {
        shutdown ();
    }

    Status TelemetryConsoleProject::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        Status status = console_->initialize ();

        if (!status.ok ())
        {
            rollback ();
            return status;
        }

        consoleAcquired_ = true;
        status = operatorInput_->initialize ();

        if (!status.ok ())
        {
            rollback ();
            return status;
        }

        operatorAcquired_ = true;
        status = presentation_->initialize ();

        if (!status.ok ())
        {
            rollback ();
            return status;
        }

        presentationAcquired_ = true;
        status = records_->initialize ();

        if (!status.ok ())
        {
            rollback ();
            return status;
        }

        recordsAcquired_      = true;
        clear                  ();
        initialized_          = true;
        snapshot_.initialized = true;
        snapshot_.console     = console_->output ();

        return StatusCode::Ok;
    }

    void TelemetryConsoleProject::shutdown () noexcept
    {
        rollback ();
    }

    bool TelemetryConsoleProject::initialized () const noexcept
    {
        return initialized_;
    }

    Status TelemetryConsoleProject::update (TimePoint            now,
                                            const ConsoleSource* sources,
                                            uint8_t sourceCount) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        snapshot_.operatorStatus = operatorInput_->update (now);

        const bool operatorReady = snapshot_.operatorStatus.ok ();
        const ConsoleInput input = {
            sources,
            sourceCount,
            operatorReady && operatorInput_->nextPressEvent        (),
            operatorReady && operatorInput_->acknowledgePressEvent (),
            now};
        const Status consoleStatus = console_->update (input);

        snapshot_.console            = console_->output ();
        snapshot_.presentationStatus = present          (now);

        if (consoleStatus.ok ())
        {
            snapshot_.recordStatus = record (now);
        }
        else
        {
            snapshot_.recordStatus = snapshot_.console.recordStatus;
        }

        snapshot_.console            = console_->output ();

        if (!consoleStatus.ok ())
        {
            return consoleStatus;
        }

        if (!snapshot_.operatorStatus.ok ())
        {
            return snapshot_.operatorStatus;
        }

        if (!snapshot_.presentationStatus.ok ())
        {
            return snapshot_.presentationStatus;
        }

        return snapshot_.recordStatus;
    }

    TelemetryConsoleProjectSnapshot
    TelemetryConsoleProject::snapshot () const noexcept
    {
        return snapshot_;
    }

    Status TelemetryConsoleProject::present (TimePoint now) noexcept
    {
        const Result<ConsoleSource> selected =
            console_->source (snapshot_.console.selectedSlot);

        if (!selected.ok ())
        {
            return selected.status ();
        }

        return presentation_->present (now,
                                       snapshot_.console,
                                       selected.value ());
    }

    Status TelemetryConsoleProject::record (TimePoint now) noexcept
    {
        if (snapshot_.console.recordReason == ConsoleRecordReason::None)
        {
            return snapshot_.console.recordStatus;
        }

        uint8_t recordSlot = snapshot_.console.recordSourceSlot;

        if (recordSlot >= TelemetryConsole::sourceCapacity)
        {
            recordSlot = snapshot_.console.selectedSlot;
        }

        const bool replacePending =
            !pendingRecordReady_ ||
            pendingReason_ != snapshot_.console.recordReason ||
            pendingSourceSlot_ != recordSlot;

        if (replacePending)
        {
            const Result<ConsoleSource> source = console_->source (recordSlot);

            if (!source.ok ())
            {
                console_->completeRecord (source.status (), now);
                return source.status     ();
            }

            const Status encodeStatus =
                encoder_.encode (snapshot_.console.recordAt,
                                 source.value (),
                                 snapshot_.console.health,
                                 snapshot_.console.recordReason,
                                 pendingRecord_);

            if (!encodeStatus.ok ())
            {
                console_->completeRecord (encodeStatus, now);
                return encodeStatus;
            }

            pendingReason_      = snapshot_.console.recordReason;
            pendingSourceSlot_  = recordSlot;
            pendingRecordReady_ = true;
        }

        if (!snapshot_.console.writeRecord)
        {
            return snapshot_.console.recordStatus;
        }

        const Status status = records_->append (pendingRecord_);

        const Status completion = console_->completeRecord (status, now);

        if (console_->output ().recordReason == ConsoleRecordReason::None)
        {
            pendingRecordReady_ = false;
            pendingReason_      = ConsoleRecordReason::None;
            pendingSourceSlot_  = TelemetryConsole::sourceCapacity;
        }

        return completion.ok () ? status : completion;
    }

    void TelemetryConsoleProject::rollback () noexcept
    {
        if (recordsAcquired_)
        {
            records_->shutdown ();
            recordsAcquired_ = false;
        }

        if (presentationAcquired_)
        {
            presentation_->shutdown ();
            presentationAcquired_ = false;
        }

        if (operatorAcquired_)
        {
            operatorInput_->shutdown ();
            operatorAcquired_ = false;
        }

        if (consoleAcquired_)
        {
            console_->shutdown ();
            consoleAcquired_ = false;
        }

        clear ();
    }

    void TelemetryConsoleProject::clear () noexcept
    {
        snapshot_.console            = console_->output ();
        snapshot_.operatorStatus     = StatusCode::NotInitialized;
        snapshot_.presentationStatus = StatusCode::NotInitialized;
        snapshot_.recordStatus       = StatusCode::NotInitialized;
        snapshot_.initialized        = false;
        pendingRecord_               = {};
        pendingReason_               = ConsoleRecordReason::None;
        pendingSourceSlot_           = TelemetryConsole::sourceCapacity;
        pendingRecordReady_          = false;
        initialized_                 = false;
    }
} // namespace adk
