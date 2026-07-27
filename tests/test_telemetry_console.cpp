#include "telemetry_console.h"

#include <cstdlib>
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

    void requireError (Status status, StatusCode expected, const char* message)
    {
        require (status.error () == expected, message);
    }

    TelemetryConsoleConfig configFor (const ConsoleSourceConfig* sources,
                                      uint8_t                    sourceCount)
    {
        return {sources,         sourceCount,   Duration (100),
                Duration (1000), Duration (20), 3};
    }

    ConsoleSource sourceFor (uint16_t sourceId, TelemetryKind kind)
    {
        return {sourceId,
                kind,
                SampleQuality::Valid,
                SequenceState::InOrder,
                Freshness::Fresh,
                250,
                -1,
                PacketValidity::Valid,
                StatusCode::Ok,
                true};
    }

    ConsoleInput inputFor (const ConsoleSource* sources, uint8_t sourceCount,
                           TimePoint observedAt)
    {
        return {sources, sourceCount, false, false, observedAt};
    }

    void completePending (TelemetryConsole& console, TimePoint now)
    {
        if (console.output ().writeRecord)
        {
            requireError (console.completeRecord (StatusCode::Ok, now), StatusCode::Ok,
                          "complete pending record");
        }
    }

    void testTraitsLifecycleAndCopiedConfiguration ()
    {
        static_assert (!std::is_copy_constructible<TelemetryConsole>::value,
                       "console cannot copy");
        static_assert (!std::is_copy_assignable<TelemetryConsole>::value,
                       "console cannot copy assign");
        static_assert (!std::is_move_constructible<TelemetryConsole>::value,
                       "console cannot move");
        static_assert (!std::is_move_assignable<TelemetryConsole>::value,
                       "console cannot move assign");
        static_assert (TelemetryConsole::sourceCapacity == 8,
                       "console capacity is fixed");

        ConsoleSourceConfig sourceConfigs[] = {{11, TelemetryKind::Temperature},
                                               {22, TelemetryKind::Contact}};
        const TelemetryConsoleConfig config = configFor (sourceConfigs, 2);
        TelemetryConsole             console            (config);

        sourceConfigs[0].sourceId = 99;

        require      (!console.initialized (), "construction is inert");
        require      (console.source (0).error () == StatusCode::NotInitialized,
                 "source unavailable before initialize");
        requireError (console.update (inputFor (nullptr, 0, TimePoint (0))),
                      StatusCode::NotInitialized, "update before initialize");
        requireError (console.completeRecord (StatusCode::Ok, TimePoint (0)),
                      StatusCode::NotInitialized,
                      "record completion before initialize");
        requireError (console.initialize (), StatusCode::Ok, "initialize");
        require      (console.initialized (), "initialized state");
        require      (console.output ().selectedSourceId == 11,
                 "configuration copied at construction");
        require      (console.source (0).ok (),
                 "configured source snapshot available");
        require      (console.source (0).value ().sourceId == 11,
                 "configured source identity is stable");
        require      (!console.source (0).value ().present,
                 "configured source begins missing");
        require      (console.source (2).error () == StatusCode::InvalidArgument,
                 "source slot bounds checked");
        requireError (console.initialize (), StatusCode::Ok, "repeat initialize");

        console.shutdown ();
        require          (!console.initialized (), "shutdown clears lifecycle");
        require          (console.output ().health == ConsoleHealth::Stopped,
                 "shutdown requests stopped evidence");
        require (console.output ().signal == ConsoleSignal::None,
                 "shutdown requests silence");
        require          (!console.output ().writeRecord, "shutdown clears pending record");
        console.shutdown ();
        requireError     (console.initialize (), StatusCode::Ok, "restart");
    }

    void testConfigurationFailures ()
    {
        ConsoleSourceConfig    valid[] = {{1, TelemetryKind::Temperature}};
        TelemetryConsoleConfig config  = configFor (valid, 0);
        TelemetryConsole       zero                (config);

        requireError (zero.initialize (), StatusCode::InvalidArgument,
                      "zero sources rejected");

        config = configFor           (nullptr, 1);
        TelemetryConsole nullSources (config);

        requireError (nullSources.initialize (), StatusCode::InvalidArgument,
                      "null configuration rejected");

        ConsoleSourceConfig tooMany[9] = {};

        config = configFor (tooMany, 9);

        TelemetryConsole overCapacity (config);

        requireError (overCapacity.initialize (), StatusCode::InvalidArgument,
                      "over capacity rejected");

        ConsoleSourceConfig duplicate[] = {{1, TelemetryKind::Temperature},
                                           {1, TelemetryKind::Contact}};

        config = configFor (duplicate, 2);

        TelemetryConsole duplicateIds (config);

        requireError (duplicateIds.initialize (), StatusCode::InvalidArgument,
                      "duplicate configured identity rejected");

        valid[0].kind = static_cast<TelemetryKind> (255);
        config        = configFor (valid, 1);

        TelemetryConsole badKind (config);

        requireError (badKind.initialize (), StatusCode::InvalidArgument,
                      "invalid configured kind rejected");

        valid[0].kind          = TelemetryKind::Temperature;
        config                 = configFor (valid, 1);
        config.heartbeatPeriod = Duration  (0);
        TelemetryConsole noHeartbeat       (config);

        requireError (noHeartbeat.initialize (), StatusCode::InvalidArgument,
                      "zero heartbeat rejected");

        config              = configFor (valid, 1);
        config.startupGrace = Duration  (0x80000000U);
        TelemetryConsole longStartup    (config);

        requireError (longStartup.initialize (), StatusCode::InvalidArgument,
                      "ambiguous startup duration rejected");

        config                       = configFor (valid, 1);
        config.maximumRecordAttempts = 0;
        TelemetryConsole noRecordAttempts (config);

        requireError (noRecordAttempts.initialize (), StatusCode::InvalidArgument,
                      "zero record attempts rejected");
    }

    void testSourceOrderRecoveryAndHealthPrecedence ()
    {
        const ConsoleSourceConfig sourceConfigs[] = {
            {10, TelemetryKind::Temperature},
            {20, TelemetryKind::RelativeHumidity},
            {30, TelemetryKind::Contact}};
        TelemetryConsole console (configFor (sourceConfigs, 3));

        requireError (console.initialize (), StatusCode::Ok, "health initialize");
        requireError (console.update (inputFor (nullptr, 0, TimePoint (0))),
                      StatusCode::Ok, "empty startup set");
        require (console.output ().health == ConsoleHealth::Starting,
                 "missing sources start within grace");
        completePending (console, TimePoint (0));

        ConsoleSource observations[] = {
            sourceFor (30, TelemetryKind::Contact),
            sourceFor (10, TelemetryKind::Temperature),
            sourceFor (20, TelemetryKind::RelativeHumidity)};

        requireError (console.update (inputFor (observations, 3, TimePoint (1))),
                      StatusCode::Ok, "arrival order independent");
        require (console.output ().health == ConsoleHealth::Healthy,
                 "all configured sources healthy");
        require (console.output ().recordSourceSlot == 2,
                 "configured slot order selects record source");
        completePending (console, TimePoint (1));

        observations[2].freshness = Freshness::Aging;
        requireError (console.update (inputFor (observations, 3, TimePoint (2))),
                      StatusCode::Ok, "aging observation");
        require         (console.output ().health == ConsoleHealth::Degraded, "aging degrades");
        completePending (console, TimePoint (2));

        observations[0].sequenceState = SequenceState::Gap;
        observations[2].freshness     = Freshness::Fresh;
        requireError (console.update (inputFor (observations, 3, TimePoint (3))),
                      StatusCode::Ok, "sequence gap");
        require         (console.output ().health == ConsoleHealth::Degraded, "gap degrades");
        completePending (console, TimePoint (3));

        observations[1].freshness = Freshness::Stale;
        requireError (console.update (inputFor (observations, 3, TimePoint (4))),
                      StatusCode::Ok, "stale observation");
        require (console.output ().health == ConsoleHealth::Fault,
                 "fault dominates degraded");
        require (console.output ().signal == ConsoleSignal::Attention,
                 "new fault requests attention");
        completePending (console, TimePoint (4));

        observations[1].freshness     = Freshness::Fresh;
        observations[0].sequenceState = SequenceState::InOrder;
        requireError (console.update (inputFor (observations, 2, TimePoint (5))),
                      StatusCode::Ok, "omitted configured source");
        require (console.output ().health == ConsoleHealth::Fault,
                 "missing source faults after startup completed");

        requireError (console.update (inputFor (observations, 2, TimePoint (100))),
                      StatusCode::Ok, "startup grace boundary");
        require (console.output ().health == ConsoleHealth::Fault,
                 "missing source faults at grace");
    }

    void testFaultClasses ()
    {
        const ConsoleSourceConfig sourceConfig[] = {{7, TelemetryKind::Counter}};

        for (uint8_t quality = static_cast<uint8_t> (SampleQuality::SensorFault);
             quality <= static_cast<uint8_t> (SampleQuality::StaleAtSource); ++quality)
        {
            TelemetryConsole console            (configFor (sourceConfig, 1));
            ConsoleSource    source = sourceFor (7, TelemetryKind::Counter);

            source.quality = static_cast<SampleQuality> (quality);
            requireError (console.initialize (), StatusCode::Ok, "quality init");
            requireError (console.update (inputFor (&source, 1, TimePoint (0))),
                          StatusCode::Ok, "quality update");
            require (console.output ().health == ConsoleHealth::Fault,
                     "invalid quality faults");
        }

        const SequenceState faultSequences[] = {SequenceState::Duplicate,
                                                SequenceState::Reordered};

        for (const SequenceState sequence : faultSequences)
        {
            TelemetryConsole console            (configFor (sourceConfig, 1));
            ConsoleSource    source = sourceFor (7, TelemetryKind::Counter);

            source.sequenceState = sequence;
            requireError (console.initialize (), StatusCode::Ok, "sequence init");
            requireError (console.update (inputFor (&source, 1, TimePoint (0))),
                          StatusCode::Ok, "sequence update");
            require (console.output ().health == ConsoleHealth::Fault,
                     "duplicate and reordered fault");
        }

        for (uint8_t validity = static_cast<uint8_t> (PacketValidity::BadVersion);
             validity <= static_cast<uint8_t> (PacketValidity::TrailingData);
             ++validity)
        {
            TelemetryConsole console            (configFor (sourceConfig, 1));
            ConsoleSource    source = sourceFor (7, TelemetryKind::Counter);

            source.packetValidity = static_cast<PacketValidity> (validity);
            requireError (console.initialize (), StatusCode::Ok, "packet init");
            requireError (console.update (inputFor (&source, 1, TimePoint (0))),
                          StatusCode::Ok, "packet update");
            require (console.output ().health == ConsoleHealth::Fault,
                     "packet corruption faults");
        }

        TelemetryConsole console            (configFor (sourceConfig, 1));
        ConsoleSource    source = sourceFor (7, TelemetryKind::Counter);

        source.observationStatus = StatusCode::HardwareFailure;
        requireError (console.initialize (), StatusCode::Ok, "status init");
        requireError (console.update (inputFor (&source, 1, TimePoint (0))),
                      StatusCode::Ok, "status update");
        require (console.output ().health == ConsoleHealth::Fault,
                 "observation failure faults");
    }

    void testInvalidInputDoesNotEscapeBounds ()
    {
        const ConsoleSourceConfig sourceConfigs[] = {{1, TelemetryKind::Temperature},
                                                     {2, TelemetryKind::Contact}};
        TelemetryConsole          console            (configFor (sourceConfigs, 2));
        ConsoleSource             source = sourceFor (1, TelemetryKind::Temperature);

        requireError (console.initialize (), StatusCode::Ok, "invalid input init");

        ConsoleInput input = inputFor (nullptr, 1, TimePoint (0));

        requireError (console.update (input), StatusCode::InvalidArgument,
                      "null input rejected");
        require (console.output ().health == ConsoleHealth::Fault,
                 "invalid input visible");

        input = inputFor (&source, 9, TimePoint (1));
        requireError     (console.update (input), StatusCode::InvalidArgument,
                      "over-capacity input rejected");

        source.sourceId = 3;
        input           = inputFor (&source, 1, TimePoint (2));
        requireError               (console.update (input), StatusCode::InvalidArgument,
                      "unknown source rejected");

        ConsoleSource duplicates[] = {sourceFor (1, TelemetryKind::Temperature),
                                      sourceFor (1, TelemetryKind::Temperature)};

        input = inputFor (duplicates, 2, TimePoint (3));
        requireError     (console.update (input), StatusCode::InvalidArgument,
                      "duplicate input identity rejected");

        source = sourceFor (1, TelemetryKind::Contact);
        input  = inputFor  (&source, 1, TimePoint (4));
        requireError       (console.update (input), StatusCode::InvalidArgument,
                      "configured kind mismatch rejected");

        source                 = sourceFor (1, TelemetryKind::Temperature);
        source.decimalExponent = 10;
        input                  = inputFor (&source, 1, TimePoint (5));
        requireError                      (console.update (input), StatusCode::InvalidArgument,
                      "scale overflow rejected");
    }

    void testSelectionAcknowledgementAndReannouncement ()
    {
        const ConsoleSourceConfig sourceConfigs[] = {{101, TelemetryKind::Temperature},
                                                     {202, TelemetryKind::Contact}};
        TelemetryConsole          console    (configFor (sourceConfigs, 2));
        ConsoleSource sources[] = {sourceFor (101, TelemetryKind::Temperature),
                                   sourceFor (202, TelemetryKind::Contact)};

        requireError (console.initialize (), StatusCode::Ok, "operator init");
        requireError (console.update (inputFor (sources, 2, TimePoint (0))),
                      StatusCode::Ok, "operator baseline");
        completePending (console, TimePoint (0));

        ConsoleInput input = inputFor (sources, 2, TimePoint (1));

        input.nextPressEvent = true;
        requireError (console.update (input), StatusCode::Ok, "next event");
        require      (console.output ().selectedSlot == 1,
                 "selection advances by configured slot");
        require (console.output ().selectedSourceId == 202,
                 "selection exposes distinct source id");

        input.observedAt = TimePoint (2);
        requireError                 (console.update (input), StatusCode::Ok, "second next event");
        require                      (console.output ().selectedSlot == 0, "events wrap selection");

        sources[0].freshness = Freshness::Stale;
        input                = inputFor (sources, 2, TimePoint (3));
        requireError                    (console.update (input), StatusCode::Ok, "fault event");
        require                         (console.output ().signal == ConsoleSignal::Attention,
                 "fault announces");
        completePending (console, TimePoint (3));

        input.acknowledgePressEvent = true;
        input.observedAt            = TimePoint (4);
        requireError                            (console.update (input), StatusCode::Ok, "acknowledge fault");
        require                                 (console.output ().health == ConsoleHealth::Fault,
                 "acknowledge preserves health");
        require (console.output ().signal == ConsoleSignal::Notice,
                 "acknowledge silences attention");
        require (console.output ().recordReason == ConsoleRecordReason::Acknowledgement,
                 "acknowledgement is recorded");
        completePending (console, TimePoint (4));

        sources[0].freshness = Freshness::Fresh;
        input                = inputFor (sources, 2, TimePoint (5));
        requireError                    (console.update (input), StatusCode::Ok, "recover");
        completePending                 (console, TimePoint (5));

        sources[1].quality = SampleQuality::SensorFault;
        input              = inputFor (sources, 2, TimePoint (6));
        requireError                  (console.update (input), StatusCode::Ok, "new fault");
        require                       (console.output ().signal == ConsoleSignal::Attention,
                 "new fault reannounces");

        input.nextPressEvent        = true;
        input.acknowledgePressEvent = true;
        input.observedAt            = TimePoint (7);
        requireError                            (console.update (input), StatusCode::InvalidArgument,
                      "invalid chord rejected");
    }

    void testRecordCoalescingRetryAndHeartbeat ()
    {
        const ConsoleSourceConfig sourceConfig[] = {{1, TelemetryKind::Temperature}};
        TelemetryConsole          console            (configFor (sourceConfig, 1));
        ConsoleSource             source = sourceFor (1, TelemetryKind::Temperature);

        requireError (console.initialize (), StatusCode::Ok, "record init");
        requireError (console.update (inputFor (&source, 1, TimePoint (100))),
                      StatusCode::Ok, "record baseline");
        require (console.output ().writeRecord, "first observation requests record");
        require (console.output ().recordReason ==
                     ConsoleRecordReason::HealthTransition,
                 "health transition has fixed priority");
        require (console.output ().recordAttempt == 1, "first record attempt");

        source.value = 260;
        requireError (console.update (inputFor (&source, 1, TimePoint (101))),
                      StatusCode::Ok, "coalesced observation");
        require (console.output ().recordReason ==
                     ConsoleRecordReason::HealthTransition,
                 "lower priority trigger cannot replace pending reason");

        requireError (
            console.completeRecord (StatusCode::HardwareFailure, TimePoint (101)),
            StatusCode::HardwareFailure, "first record failure");
        require (!console.output ().writeRecord, "retry waits");
        require (console.output ().recordStatus.error () == StatusCode::HardwareFailure,
                 "record failure remains visible");

        requireError (console.update (inputFor (&source, 1, TimePoint (120))),
                      StatusCode::Ok, "before retry");
        require (!console.output ().writeRecord, "retry not early");

        requireError (console.update (inputFor (&source, 1, TimePoint (121))),
                      StatusCode::Ok, "exact retry");
        require (console.output ().writeRecord, "retry at deadline");
        require (console.output ().recordAttempt == 2, "second bounded attempt");

        requireError (
            console.completeRecord (StatusCode::HardwareFailure, TimePoint (121)),
            StatusCode::HardwareFailure, "second record failure");
        requireError (console.update (inputFor (&source, 1, TimePoint (141))),
                      StatusCode::Ok, "third retry");
        require      (console.output ().recordAttempt == 3, "third bounded attempt");
        requireError (
            console.completeRecord (StatusCode::HardwareFailure, TimePoint (141)),
            StatusCode::HardwareFailure, "last record failure");
        require (!console.output ().writeRecord, "retry exhausted");
        require (console.output ().recordReason == ConsoleRecordReason::None,
                 "exhaustion clears bounded pending state");

        source.value = 270;
        requireError (console.update (inputFor (&source, 1, TimePoint (142))),
                      StatusCode::Ok, "new observation after exhaustion");
        require (console.output ().recordReason == ConsoleRecordReason::Observation,
                 "new work can follow exhausted work");
        requireError (console.completeRecord (StatusCode::Ok, TimePoint (142)),
                      StatusCode::Ok, "record recovery");

        requireError (console.update (inputFor (&source, 1, TimePoint (1141))),
                      StatusCode::Ok, "before heartbeat");
        require      (!console.output ().writeRecord, "heartbeat not early");
        requireError (console.update (inputFor (&source, 1, TimePoint (1142))),
                      StatusCode::Ok, "exact heartbeat");
        require (console.output ().recordReason == ConsoleRecordReason::Heartbeat,
                 "heartbeat scheduled deterministically");
    }

    void testTimestampWrapAndDeterministicReplay ()
    {
        const ConsoleSourceConfig sourceConfig[] = {{5, TelemetryKind::Distance}};
        TelemetryConsoleConfig    config         = configFor (sourceConfig, 1);
        TelemetryConsole          first                      (config);
        TelemetryConsole          second                     (config);
        ConsoleSource             source = sourceFor         (5, TelemetryKind::Distance);
        const TimePoint times[] = {TimePoint                 (0xfffffff0U), TimePoint (0xfffffff5U),
                                   TimePoint (0x00000004U), TimePoint (0x000003d8U)};

        requireError (first.initialize (), StatusCode::Ok, "first replay init");
        requireError (second.initialize (), StatusCode::Ok, "second replay init");

        for (const TimePoint now : times)
        {
            const ConsoleInput input = inputFor (&source, 1, now);

            requireError (first.update (input), StatusCode::Ok, "first replay");
            requireError (second.update (input), StatusCode::Ok, "second replay");

            const ConsoleOutput left  = first.output  ();
            const ConsoleOutput right = second.output ();

            require (left.health == right.health, "replay health identical");
            require (left.signal == right.signal, "replay signal identical");
            require (left.recordReason == right.recordReason,
                     "replay record reason identical");
            require (left.writeRecord == right.writeRecord, "replay request identical");

            if (left.writeRecord)
            {
                requireError (first.completeRecord (StatusCode::Ok, now),
                              StatusCode::Ok, "first replay completion");
                requireError (second.completeRecord (StatusCode::Ok, now),
                              StatusCode::Ok, "second replay completion");
            }
        }

        const ConsoleInput reversed = inputFor (&source, 1, TimePoint (0x800003d8U));

        requireError (first.update (reversed), StatusCode::InvalidArgument,
                      "ambiguous time reversal rejected");
        require (first.output ().health == ConsoleHealth::Fault,
                 "timing fault visible");
        require (!first.output ().writeRecord,
                 "rejected time does not request invalid evidence");
    }

    void testCapacityOrderingAndFaultIdentity ()
    {
        ConsoleSourceConfig sourceConfigs[TelemetryConsole::sourceCapacity];
        ConsoleSource       forward[TelemetryConsole::sourceCapacity];
        ConsoleSource       reverse[TelemetryConsole::sourceCapacity];

        for (uint8_t slot = 0; slot < TelemetryConsole::sourceCapacity; ++slot)
        {
            sourceConfigs[slot] = {
                static_cast<uint16_t> (100U + slot),
                TelemetryKind::Counter};
            forward[slot] = sourceFor (sourceConfigs[slot].sourceId,
                                       sourceConfigs[slot].kind);
            reverse[TelemetryConsole::sourceCapacity - slot - 1U] = forward[slot];
        }

        TelemetryConsoleConfig config =
            configFor (sourceConfigs, TelemetryConsole::sourceCapacity);
        TelemetryConsole left  (config);
        TelemetryConsole right (config);

        requireError (left.initialize (), StatusCode::Ok, "capacity left init");
        requireError (right.initialize (), StatusCode::Ok, "capacity right init");
        requireError (left.update (inputFor (forward,
                                             TelemetryConsole::sourceCapacity,
                                             TimePoint (0))),
                      StatusCode::Ok,
                      "capacity forward update");
        requireError (right.update (inputFor (reverse,
                                              TelemetryConsole::sourceCapacity,
                                              TimePoint (0))),
                      StatusCode::Ok,
                      "capacity reverse update");

        const ConsoleOutput forwardOutput = left.output  ();
        const ConsoleOutput reverseOutput = right.output ();

        require (forwardOutput.health == ConsoleHealth::Healthy,
                 "maximum capacity healthy");
        require (forwardOutput.health == reverseOutput.health,
                 "ordering keeps health");
        require (forwardOutput.recordReason == reverseOutput.recordReason,
                 "ordering keeps reason");
        require (forwardOutput.recordSourceSlot == reverseOutput.recordSourceSlot,
                 "ordering keeps record slot");
        require (forwardOutput.recordAttempt == reverseOutput.recordAttempt,
                 "ordering keeps attempt");

        completePending (left, TimePoint (0));

        forward[0].freshness = Freshness::Stale;
        ConsoleInput input = inputFor (forward,
                                       TelemetryConsole::sourceCapacity,
                                       TimePoint (1));

        input.acknowledgePressEvent = true;
        requireError (left.update (input), StatusCode::Ok, "fault with ack");
        require      (left.output ().signal == ConsoleSignal::Attention,
                 "new fault dominates simultaneous acknowledgement");
        completePending (left, TimePoint (1));

        input.acknowledgePressEvent = true;
        input.observedAt            = TimePoint (2);
        requireError                            (left.update (input), StatusCode::Ok, "fault ack");
        require                                 (left.output ().signal == ConsoleSignal::Notice,
                 "known fault can be acknowledged");
        completePending (left, TimePoint (2));

        forward[0].freshness = Freshness::Fresh;
        forward[1].quality   = SampleQuality::SensorFault;
        input                = inputFor (forward,
                         TelemetryConsole::sourceCapacity,
                         TimePoint (3));
        requireError (left.update (input), StatusCode::Ok, "changed fault identity");
        require      (left.output ().signal == ConsoleSignal::Attention,
                 "changed fault reannounces while faulted");
    }

    void testRecordCompletionChronology ()
    {
        const ConsoleSourceConfig sourceConfig[] = {
            {1, TelemetryKind::Temperature}};
        TelemetryConsole console         (configFor (sourceConfig, 1));
        ConsoleSource source = sourceFor (1, TelemetryKind::Temperature);

        requireError (console.initialize (), StatusCode::Ok, "chronology init");
        requireError (console.update (inputFor (&source, 1, TimePoint (100))),
                      StatusCode::Ok,
                      "chronology update");
        requireError (console.completeRecord (StatusCode::Ok, TimePoint (99)),
                      StatusCode::InvalidArgument,
                      "backward completion rejected");
        require (console.output ().writeRecord,
                 "rejected completion preserves pending request");
        requireError (console.completeRecord (StatusCode::Ok, TimePoint (100)),
                      StatusCode::Ok,
                      "same-time completion allowed");

        source.value = 251;
        requireError (console.update (inputFor (&source, 1, TimePoint (101))),
                      StatusCode::Ok,
                      "update after completion");
        requireError (console.completeRecord (StatusCode::HardwareFailure,
                                              TimePoint (0xfffffff0U)),
                      StatusCode::InvalidArgument,
                      "ambiguous completion rejected");
    }
} // namespace

int main ()
{
    testTraitsLifecycleAndCopiedConfiguration     ();
    testConfigurationFailures                     ();
    testSourceOrderRecoveryAndHealthPrecedence    ();
    testFaultClasses                              ();
    testInvalidInputDoesNotEscapeBounds           ();
    testSelectionAcknowledgementAndReannouncement ();
    testRecordCoalescingRetryAndHeartbeat         ();
    testTimestampWrapAndDeterministicReplay       ();
    testCapacityOrderingAndFaultIdentity          ();
    testRecordCompletionChronology                ();

    std::cout << "telemetry console tests passed\n";
    return 0;
}
