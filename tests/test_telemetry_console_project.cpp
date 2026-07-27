#include "telemetry_console_project.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>

namespace {

    using namespace adk;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    struct FakeOperator final : TelemetryConsoleOperator
    {
        Status   initializeStatus = StatusCode::Ok;
        Status   updateStatus     = StatusCode::Ok;
        uint32_t initializeCount  = 0;
        uint32_t shutdownCount    = 0;
        uint32_t updateCount      = 0;
        bool     next             = false;
        bool     acknowledge      = false;
        bool     active           = false;

        ~FakeOperator () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            ++shutdownCount;
            active = false;
        }

        bool initialized () const noexcept override
        {
            return active;
        }

        Status update (TimePoint) noexcept override
        {
            ++updateCount;
            return updateStatus;
        }

        bool nextPressEvent () const noexcept override
        {
            return next;
        }

        bool acknowledgePressEvent () const noexcept override
        {
            return acknowledge;
        }
    };

    struct FakePresentation final : TelemetryConsolePresentation
    {
        Status        initializeStatus = StatusCode::Ok;
        Status        presentStatus    = StatusCode::Ok;
        ConsoleOutput shown            = {};
        ConsoleSource selected         = {};
        uint32_t      initializeCount  = 0;
        uint32_t      shutdownCount    = 0;
        uint32_t      presentCount     = 0;
        bool          active           = false;

        ~FakePresentation () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            ++shutdownCount;
            active = false;
        }

        bool initialized () const noexcept override
        {
            return active;
        }

        Status present (TimePoint, const ConsoleOutput& output,
                        const ConsoleSource& source) noexcept override
        {
            ++presentCount;
            shown    = output;
            selected = source;
            return presentStatus;
        }
    };

    struct FakeRecordSink final : RecordSink
    {
        Status       initializeStatus = StatusCode::Ok;
        Status       appendStatus     = StatusCode::Ok;
        StableRecord last             = {};
        StableRecord attempts[4]      = {};
        uint32_t     initializeCount  = 0;
        uint32_t     shutdownCount    = 0;
        uint32_t     appendCount      = 0;
        bool         active           = false;

        ~FakeRecordSink () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            ++shutdownCount;
            active = false;
        }

        Status append (const StableRecord& record) noexcept override
        {
            ++appendCount;
            if (appendCount <= 4)
            {
                attempts[appendCount - 1U] = record;
            }

            if (appendStatus.ok ())
            {
                last = record;
            }

            return appendStatus;
        }

        bool initialized () const noexcept override
        {
            return active;
        }
    };

    TelemetryConsoleConfig consoleConfig (
        const ConsoleSourceConfig* sources) noexcept
    {
        return {sources,
                2,
                Duration (100),
                Duration (1000),
                Duration (20),
                3};
    }

    ConsoleSource sourceFor (uint16_t sourceId, TelemetryKind kind) noexcept
    {
        return {sourceId,
                kind,
                SampleQuality::Valid,
                SequenceState::InOrder,
                Freshness::Fresh,
                42,
                0,
                PacketValidity::Valid,
                StatusCode::Ok,
                true};
    }

    void testTraitsAndLifecycle ()
    {
        static_assert (
            !std::is_copy_constructible<TelemetryConsoleProject>::value,
            "project cannot copy");
        static_assert (!std::is_move_constructible<TelemetryConsoleProject>::value,
                       "project cannot move");

        const ConsoleSourceConfig configs[] = {
            {1, TelemetryKind::Temperature},
            {2, TelemetryKind::Contact}};
        TelemetryConsole  console (consoleConfig (configs));
        FakeOperator      operatorInput;
        FakePresentation  presentation;
        FakeRecordSink    records;
        TelemetryConsoleProject project (
            console, operatorInput, presentation, records);

        require (!project.initialized (), "construction is inert");
        require (project.update (TimePoint (0), nullptr, 0).error () ==
                     StatusCode::NotInitialized,
                 "update before initialize");
        require (project.initialize ().ok (), "initialize");
        require (project.initialized (), "initialized state");
        require (console.initialized (), "console acquired");
        require (operatorInput.active, "operator acquired");
        require (presentation.active, "presentation acquired");
        require (records.active, "records acquired");
        require (project.initialize ().ok (), "repeat initialize");
        require (operatorInput.initializeCount == 1, "operator acquired once");

        project.shutdown ();
        require          (!project.initialized (), "shutdown clears lifecycle");
        require          (!console.initialized (), "console stopped");
        require          (!operatorInput.active && !presentation.active && !records.active,
                 "dependencies released");
        require (project.snapshot ().console.health == ConsoleHealth::Stopped,
                 "shutdown requests stopped presentation");
        project.shutdown ();
        require          (project.initialize ().ok (), "restart");
    }

    void testEveryInitializationRollback ()
    {
        for (uint8_t failure = 0; failure < 3; ++failure)
        {
            const ConsoleSourceConfig configs[] = {
                {1, TelemetryKind::Temperature},
                {2, TelemetryKind::Contact}};
            TelemetryConsole console (consoleConfig (configs));
            FakeOperator     operatorInput;
            FakePresentation presentation;
            FakeRecordSink   records;

            if (failure == 0)
            {
                operatorInput.initializeStatus = StatusCode::ResourceBusy;
            }
            else if (failure == 1)
            {
                presentation.initializeStatus = StatusCode::HardwareFailure;
            }
            else
            {
                records.initializeStatus = StatusCode::HardwareFailure;
            }

            TelemetryConsoleProject project (
                console, operatorInput, presentation, records);

            require (!project.initialize ().ok (), "injected initialize failure");
            require (!project.initialized (), "failed project remains inert");
            require (!console.initialized (), "failure rolls back console");
            require (!operatorInput.active && !presentation.active && !records.active,
                     "failure rolls back all seams");
            require (operatorInput.shutdownCount == (failure == 0 ? 0U : 1U),
                     "operator rollback matches acquisition");
            require (presentation.shutdownCount == (failure < 2 ? 0U : 1U),
                     "presentation rollback matches acquisition");
            require (records.shutdownCount == 0,
                     "failed record acquisition is not owned");
        }

        const ConsoleSourceConfig invalidConfigs[] = {
            {1, static_cast<TelemetryKind> (255)},
            {2, TelemetryKind::Contact}};
        TelemetryConsole invalidConsole (consoleConfig (invalidConfigs));
        FakeOperator     unusedOperator;
        FakePresentation unusedPresentation;
        FakeRecordSink   unusedRecords;
        TelemetryConsoleProject invalidProject (
            invalidConsole, unusedOperator, unusedPresentation, unusedRecords);

        require (invalidProject.initialize ().error () ==
                     StatusCode::InvalidArgument,
                 "console initialize failure reported");
        require (unusedOperator.initializeCount == 0 &&
                     unusedPresentation.initializeCount == 0 &&
                     unusedRecords.initializeCount == 0,
                 "console failure touches no later dependency");
    }

    void testInertDestructionHasNoExternalEffects ()
    {
        const ConsoleSourceConfig configs[] = {
            {1, TelemetryKind::Temperature},
            {2, TelemetryKind::Contact}};
        TelemetryConsole console (consoleConfig (configs));
        FakeOperator     operatorInput;
        FakePresentation presentation;
        FakeRecordSink   records;

        operatorInput.active = true;
        presentation.active  = true;
        records.active       = true;

        {
            TelemetryConsoleProject project (
                console, operatorInput, presentation, records);
        }

        require (operatorInput.shutdownCount == 0,
                 "inert destruction leaves operator untouched");
        require (presentation.shutdownCount == 0,
                 "inert destruction leaves presentation untouched");
        require (records.shutdownCount == 0,
                 "inert destruction leaves records untouched");
        require (operatorInput.active && presentation.active && records.active,
                 "borrowed active seams remain active");
    }

    void testNarrativeFlowAndFailureIsolation ()
    {
        const ConsoleSourceConfig configs[] = {
            {1, TelemetryKind::Temperature},
            {2, TelemetryKind::Contact}};
        TelemetryConsole console (consoleConfig (configs));
        FakeOperator     operatorInput;
        FakePresentation presentation;
        FakeRecordSink   records;
        TelemetryConsoleProject project (
            console, operatorInput, presentation, records);
        ConsoleSource sources[] = {
            sourceFor (1, TelemetryKind::Temperature),
            sourceFor (2, TelemetryKind::Contact)};

        require (project.initialize ().ok (), "flow initialize");
        require (project.update (TimePoint (10), sources, 2).ok (),
                 "healthy flow");
        require (project.snapshot ().console.health == ConsoleHealth::Healthy,
                 "console decides healthy");
        require (presentation.shown.health == ConsoleHealth::Healthy,
                 "presentation sees decision");
        require (presentation.selected.sourceId == 1,
                 "presentation sees selected source");
        require (records.appendCount == 1, "health transition recorded");
        require (std::memcmp (records.last.text, "TEL1,10,2,C,V,I,F,42,0,H,H\n",
                              records.last.length) == 0,
                 "canonical record stored");

        operatorInput.next = true;
        require (project.update (TimePoint (11), sources, 2).ok (),
                 "selection event");
        require (project.snapshot ().console.selectedSourceId == 2,
                 "operator advances selection");
        operatorInput.next = false;

        presentation.presentStatus = StatusCode::HardwareFailure;
        sources[0].freshness       = Freshness::Aging;
        require (project.update (TimePoint (12), sources, 2).error () ==
                     StatusCode::HardwareFailure,
                 "presentation failure reported");
        require (project.snapshot ().console.health == ConsoleHealth::Degraded,
                 "presentation failure does not rewrite observation health");

        presentation.presentStatus = StatusCode::Ok;
        records.appendStatus       = StatusCode::HardwareFailure;
        sources[0].freshness       = Freshness::Stale;
        require (project.update (TimePoint (13), sources, 2).error () ==
                     StatusCode::HardwareFailure,
                 "record failure reported");
        require (project.snapshot ().console.health == ConsoleHealth::Fault,
                 "record failure does not rewrite source fault");
        require (project.snapshot ().console.recordStatus.error () ==
                     StatusCode::HardwareFailure,
                 "record diagnostic visible");

        operatorInput.updateStatus = StatusCode::HardwareFailure;
        sources[0].freshness       = Freshness::Fresh;
        require (project.update (TimePoint (14), sources, 2).error () ==
                     StatusCode::HardwareFailure,
                 "operator failure reported");
        require (project.snapshot ().console.health == ConsoleHealth::Healthy,
                 "operator failure does not rewrite source health");

        presentation.presentStatus = StatusCode::HardwareFailure;
        require (project.update (TimePoint (33), sources, 2).error () ==
                     StatusCode::HardwareFailure,
                 "simultaneous failures preserve precedence");
        const TelemetryConsoleProjectSnapshot failed = project.snapshot ();

        require (failed.operatorStatus.error () == StatusCode::HardwareFailure,
                 "operator diagnostic retained");
        require (failed.presentationStatus.error () ==
                     StatusCode::HardwareFailure,
                 "presentation diagnostic retained");
        require (failed.recordStatus.error () == StatusCode::HardwareFailure,
                 "record diagnostic retained");
        require (failed.console.health == ConsoleHealth::Healthy,
                 "seam failures do not corrupt console decision");
    }

    void testBoundedRetryAndRestart ()
    {
        const ConsoleSourceConfig configs[] = {
            {1, TelemetryKind::Temperature},
            {2, TelemetryKind::Contact}};
        TelemetryConsole console (consoleConfig (configs));
        FakeOperator     operatorInput;
        FakePresentation presentation;
        FakeRecordSink   records;
        TelemetryConsoleProject project (
            console, operatorInput, presentation, records);
        ConsoleSource sources[] = {
            sourceFor (1, TelemetryKind::Temperature),
            sourceFor (2, TelemetryKind::Contact)};

        records.appendStatus = StatusCode::HardwareFailure;
        require (project.initialize ().ok (), "retry initialize");
        require (!project.update (TimePoint (0), sources, 2).ok (),
                 "first append fails");
        require (records.appendCount == 1, "first append attempted");
        require (!project.update (TimePoint (19), sources, 2).ok (),
                 "failure remains visible before retry");
        require (records.appendCount == 1, "retry is not early");
        sources[0].value = 99;
        require (!project.update (TimePoint (20), sources, 2).ok (),
                 "second append fails");
        require (records.appendCount == 2, "retry at boundary");
        require (records.attempts[0].length == records.attempts[1].length,
                 "retry length is stable");
        require (std::memcmp (records.attempts[0].text,
                              records.attempts[1].text,
                              records.attempts[0].length) == 0,
                 "retry bytes are stable across observation changes");
        require (!project.update (TimePoint (40), sources, 2).ok (),
                 "third append fails");
        require (records.appendCount == 3, "attempt count bounded");
        require (!project.update (TimePoint (60), sources, 2).ok (),
                 "terminal failure remains visible");
        require (records.appendCount == 3, "no fourth retry");

        project.shutdown ();
        records.appendStatus = StatusCode::Ok;
        require (project.initialize ().ok (), "restart after retry exhaustion");
        require (project.update (TimePoint (0), sources, 2).ok (),
                 "restart clears pending failure");
        require (records.appendCount == 4, "restart records fresh transition");
    }

    void testHigherPriorityRecordReplacement ()
    {
        const ConsoleSourceConfig configs[] = {
            {1, TelemetryKind::Temperature},
            {2, TelemetryKind::Contact}};
        TelemetryConsole console (consoleConfig (configs));
        FakeOperator     operatorInput;
        FakePresentation presentation;
        FakeRecordSink   records;
        TelemetryConsoleProject project (
            console, operatorInput, presentation, records);
        ConsoleSource sources[] = {
            sourceFor (1, TelemetryKind::Temperature),
            sourceFor (2, TelemetryKind::Contact)};

        require (project.initialize ().ok (), "replacement initialize");
        require (project.update (TimePoint (0), sources, 2).ok (),
                 "replacement baseline");

        records.appendStatus = StatusCode::HardwareFailure;
        sources[0].value     = 43;
        require (!project.update (TimePoint (1), sources, 2).ok (),
                 "observation record fails");

        sources[0].freshness = Freshness::Stale;
        require (!project.update (TimePoint (2), sources, 2).ok (),
                 "higher priority fault coalesces");
        require (records.appendCount == 2,
                 "higher priority waits for retry deadline");

        sources[0].value = 44;
        require (!project.update (TimePoint (3), sources, 2).ok (),
                 "later lower-priority observation coalesces");

        require (!project.update (TimePoint (21), sources, 2).ok (),
                 "replacement retry fails");
        require (records.appendCount == 3, "replacement attempted");
        require (records.attempts[1].length != records.attempts[2].length ||
                     std::memcmp (records.attempts[1].text,
                                  records.attempts[2].text,
                                  records.attempts[1].length) != 0,
                 "higher priority event replaces pending bytes");
        require (std::memcmp (records.attempts[2].text,
                              "TEL1,2,1,T,V,I,S,43,0,F,H\n",
                              records.attempts[2].length) == 0,
                 "replacement keeps event timestamp and reason");

        require (!project.update (TimePoint (41), sources, 2).ok (),
                 "replacement retry repeats");
        require (records.appendCount == 4, "replacement second attempt");
        require (records.attempts[2].length == records.attempts[3].length,
                 "replacement retry length stable");
        require (std::memcmp (records.attempts[2].text,
                              records.attempts[3].text,
                              records.attempts[2].length) == 0,
                 "replacement retry bytes stable");
    }

    void testActiveDestructionChronologyAndReplay ()
    {
        const ConsoleSourceConfig configs[] = {
            {1, TelemetryKind::Temperature},
            {2, TelemetryKind::Contact}};
        FakeOperator     operatorInput;
        FakePresentation presentation;
        FakeRecordSink   records;

        {
            TelemetryConsole console        (consoleConfig (configs));
            TelemetryConsoleProject project (
                console, operatorInput, presentation, records);

            require (project.initialize ().ok (), "active destruction initialize");
        }

        require (operatorInput.shutdownCount == 1 &&
                     presentation.shutdownCount == 1 &&
                     records.shutdownCount == 1,
                 "active destruction releases acquired seams");

        TelemetryConsole firstConsole  (consoleConfig (configs));
        TelemetryConsole secondConsole (consoleConfig (configs));
        FakeOperator     firstOperator;
        FakeOperator     secondOperator;
        FakePresentation firstPresentation;
        FakePresentation secondPresentation;
        FakeRecordSink   firstRecords;
        FakeRecordSink   secondRecords;
        TelemetryConsoleProject first (
            firstConsole, firstOperator, firstPresentation, firstRecords);
        TelemetryConsoleProject second (
            secondConsole, secondOperator, secondPresentation, secondRecords);
        ConsoleSource sources[] = {
            sourceFor (1, TelemetryKind::Temperature),
            sourceFor (2, TelemetryKind::Contact)};

        require (first.initialize ().ok () && second.initialize ().ok (),
                 "replay projects initialize");

        const TimePoint times[] = {
            TimePoint (100), TimePoint (101), TimePoint (1100)};

        for (uint8_t index = 0; index < 3; ++index)
        {
            sources[0].value = static_cast<int32_t> (42 + index);
            require (first.update (times[index], sources, 2).ok (),
                     "first project replay");
            require (second.update (times[index], sources, 2).ok (),
                     "second project replay");
            require (first.snapshot ().console.health ==
                         second.snapshot ().console.health,
                     "project replay health identical");
            require (firstRecords.last.length == secondRecords.last.length,
                     "project replay record length identical");
            require (std::memcmp (firstRecords.last.text,
                                  secondRecords.last.text,
                                  firstRecords.last.length) == 0,
                     "project replay bytes identical");
        }

        const uint32_t appendCount = firstRecords.appendCount;

        require (first.update (TimePoint (1099), sources, 2).error () ==
                     StatusCode::InvalidArgument,
                 "coordinator rejects backward time");
        require (first.snapshot ().console.health == ConsoleHealth::Fault,
                 "coordinator exposes chronology fault");
        require (firstRecords.appendCount == appendCount,
                 "rejected chronology writes no durable record");
        require (!first.snapshot ().console.writeRecord,
                 "rejected chronology leaves no write request");
        require (first.update (TimePoint (1101), sources, 2).ok (),
                 "valid time recovers after chronology fault");
        require (firstRecords.appendCount == appendCount + 1U,
                 "recovery records one health transition only");
    }
} // namespace

int main ()
{
    testTraitsAndLifecycle                   ();
    testEveryInitializationRollback          ();
    testInertDestructionHasNoExternalEffects ();
    testNarrativeFlowAndFailureIsolation     ();
    testBoundedRetryAndRestart               ();
    testHigherPriorityRecordReplacement      ();
    testActiveDestructionChronologyAndReplay ();

    std::cout << "telemetry console project tests passed\n";
    return 0;
}
