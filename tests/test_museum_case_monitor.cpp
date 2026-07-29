#include <museum_case_monitor.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

// clang-format off
namespace {
    void require (bool condition, const char* message)

    {
        if (!condition)

        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);

        }
    }

    adk::MuseumCaseConfig config (uint8_t attempts = 3)

    {
        return {0x10203040UL,
                31,
                1,
                11,
                21,
                2,
                12,
                22,
                3,
                13,
                23,
                4,
                14,
                24,
                adk::Duration (100),

                adk::Duration (100),

                adk::Duration (100),

                adk::Duration (100),

                5,
                15,
                6,
                16,
                adk::Duration (100),

                adk::Duration (100),

                adk::Duration (20),

                adk::Duration (10),

                attempts};
    }

    adk::MuseumCaseEnvelope frame (uint32_t now = 100, uint32_t sequence = 1)

    {
        return {adk::TimePoint (now),

                {{1, 11, 21, sequence, adk::TimePoint (now), 800, 10, adk::Duration (2),

                  adk::Duration (100), true, adk::StatusCode::Ok},

                 0,
                 20,
                 adk::ProbeQuality::Dry,
                 adk::Duration (0),

                 adk::StatusCode::Ok},
                {{{2, 12, 22, sequence, adk::TimePoint (now), 18000, 500, false,

                   adk::StatusCode::Ok},
                  {3, 13, 23, sequence, adk::TimePoint (now), 100,

                   adk::ThresholdState::Below, false, adk::StatusCode::Ok},
                  {4, 14, 24, sequence, adk::TimePoint (now), 100,

                   adk::ThresholdState::Below, false, adk::StatusCode::Ok}},
                 adk::ThermalQuality::Normal,
                 adk::RadiantQuality::Quiet,
                 adk::Duration (0),

                 adk::Duration (0),

                 adk::Duration (0),

                 false,
                 false,
                 adk::StatusCode::Ok},
                {5,
                 15,
                 sequence,
                 {adk::MagneticSource::ContactDigital, 1, adk::Level::High,
                  adk::TimePoint (now), adk::MagneticPolarity::Unspecified, false,

                  false, true, adk::Duration (20), adk::MagneticQuality::Valid,

                  adk::StatusCode::Ok}},
                {6, 16, sequence, adk::TimePoint (now), false, adk::StatusCode::Ok},

                false,
                {0, 0, 0, 0, adk::TimePoint (), adk::MuseumCaseHealth::Qualifying, 0, 0,

                 0, 0, 0, 0, 0, 0, 0, false, adk::StatusCode::NotInitialized}};
    }

    uint8_t hazard (adk::MuseumHazard value)

    {
        return static_cast<uint8_t> (value);
    }

    bool intentEqual (const adk::MuseumCaseIntent& left,

                      const adk::MuseumCaseIntent& right)
    {
        return left.ownerToken == right.ownerToken &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.configurationRevision == right.configurationRevision &&
               left.health == right.health && left.hazardMask == right.hazardMask &&
               left.rgbBlinkCode == right.rgbBlinkCode &&
               left.lcdShowsAgeOrFault == right.lcdShowsAgeOrFault &&
               left.alarmSoundIntent == right.alarmSoundIntent &&
               left.inertRelayLampIntent == right.inertRelayLampIntent &&
               left.alarmOutputInactive == right.alarmOutputInactive;
    }

    bool auditEqual (const adk::MuseumAuditIntent& left,

                     const adk::MuseumAuditIntent& right)
    {
        return left.ownerToken == right.ownerToken &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.configurationRevision == right.configurationRevision &&
               left.recordSequence == right.recordSequence &&
               left.observedAt == right.observedAt && left.health == right.health &&
               left.hazardMask == right.hazardMask &&
               left.liquidSequence == right.liquidSequence &&
               left.thermistorSequence == right.thermistorSequence &&
               left.digitalTemperatureSequence == right.digitalTemperatureSequence &&
               left.radiantSequence == right.radiantSequence &&
               left.reedSequence == right.reedSequence &&
               left.acknowledgeSequence == right.acknowledgeSequence &&
               left.witnessDigest == right.witnessDigest &&
               left.issuedAt == right.issuedAt && left.attempt == right.attempt;
    }

    bool resultEqual (const adk::MuseumCaseResult& left,

                      const adk::MuseumCaseResult& right)
    {
        return intentEqual (left.intent, right.intent) &&

               left.hasAuditIntent == right.hasAuditIntent &&
               auditEqual (left.auditIntent, right.auditIntent) &&

               left.status == right.status;
    }

    adk::MuseumAuditReceipt receipt (const adk::MuseumAuditIntent& intent,

                                     bool                          accepted = true)
    {
        return {intent.ownerToken,
                intent.lifecycleGeneration,
                intent.configurationRevision,
                intent.recordSequence,
                intent.observedAt,
                intent.health,
                intent.hazardMask,
                intent.liquidSequence,
                intent.thermistorSequence,
                intent.digitalTemperatureSequence,
                intent.radiantSequence,
                intent.reedSequence,
                intent.acknowledgeSequence,
                intent.witnessDigest,
                intent.attempt,
                accepted,
                accepted ? adk::StatusCode::Ok : adk::StatusCode::HardwareFailure};
    }

    uint32_t digest (const adk::MuseumAuditIntent& intent)

    {
        uint32_t   hash = 0x811c9dc5UL;
        const auto byte = [&hash] (uint8_t value)
        {
            hash ^= value;
            hash *= 0x01000193UL;
        };
        const auto word16 = [&byte] (uint16_t value)
        {
            byte (static_cast<uint8_t> (value));

            byte (static_cast<uint8_t> (value >> 8U));

        };
        const auto word32 = [&byte] (uint32_t value)
        {
            byte (static_cast<uint8_t> (value));

            byte (static_cast<uint8_t> (value >> 8U));

            byte (static_cast<uint8_t> (value >> 16U));

            byte (static_cast<uint8_t> (value >> 24U));

        };
        const char domain[] = "ADK.MUSEUM.AUDIT.V1";
        for (char value : domain)

        {
            if (value != '\0')

            {
                byte (static_cast<uint8_t> (value));

            }
        }
        word32 (intent.ownerToken);

        word32 (intent.lifecycleGeneration);

        word16 (intent.configurationRevision);

        word32 (intent.recordSequence);

        word32 (intent.observedAt.milliseconds ());

        byte (static_cast<uint8_t> (intent.health));

        byte (intent.hazardMask);

        word32 (intent.liquidSequence);

        word32 (intent.thermistorSequence);

        word32 (intent.digitalTemperatureSequence);

        word32 (intent.radiantSequence);

        word32 (intent.reedSequence);

        word32 (intent.acknowledgeSequence);

        return hash;
    }

    adk::MuseumCaseResult update (adk::MuseumCaseMonitor&        monitor,

                                  const adk::MuseumCaseEnvelope& envelope)
    {
        adk::MuseumCaseResult result = {{0, 0, 0, adk::MuseumCaseHealth::Qualifying, 0,
                                         0, true, false, false, true},
                                        false,
                                        {0, 0, 0, 0, adk::TimePoint (),

                                         adk::MuseumCaseHealth::Qualifying, 0, 0, 0, 0,
                                         0, 0, 0, 0, adk::TimePoint (), 0},

                                        adk::StatusCode::NotInitialized};
        result.status                = adk::StatusCode::InternalInvariant;
        const adk::Status status     = monitor.update (envelope, result);

        require (status == result.status, "returned and published statuses agree");

        return result;
    }

    void acceptOutstanding (adk::MuseumCaseMonitor&       monitor,

                            adk::MuseumCaseEnvelope&      envelope,
                            const adk::MuseumAuditIntent& intent)
    {
        envelope.hasAuditReceipt = true;
        envelope.auditReceipt    = receipt (intent);

        require (update (monitor, envelope).status.ok (),

                 "matching accepted receipt retires record");
        envelope.hasAuditReceipt = false;
    }

    void testLifecycleAndConfiguration ()

    {
        static_assert (!std::is_copy_constructible<adk::MuseumCaseMonitor>::value,

                       "monitor does not copy");
        static_assert (!std::is_move_constructible<adk::MuseumCaseMonitor>::value,

                       "monitor does not move");
        static_assert (std::is_trivially_destructible<adk::MuseumCaseMonitor>::value,

                       "monitor destruction is inert");

        adk::MuseumCaseMonitor monitor (config ());

        require (!monitor.initialized (), "construction is inert");

        require (monitor.snapshot ().health == adk::MuseumCaseHealth::Qualifying &&

                     monitor.snapshot ().alarmOutputInactive,

                 "construction publishes canonical inactive intent");
        adk::MuseumCaseResult sentinel = update (monitor, frame ());

        sentinel.hasAuditIntent        = true;
        sentinel.status                = adk::StatusCode::InternalInvariant;
        require (monitor.update (frame (), sentinel).error () ==

                         adk::StatusCode::NotInitialized &&
                     sentinel.intent.alarmOutputInactive,
                 "pre-initialize update stays inactive");
        require (monitor.reset (adk::TimePoint (0)).error () ==

                     adk::StatusCode::NotInitialized,
                 "pre-initialize reset rejects");
        require (monitor.initialize (adk::TimePoint (90)).ok () &&

                     monitor.initialize (adk::TimePoint (91)).ok (),

                 "initialize is idempotent");
        require (monitor.snapshot ().lifecycleGeneration == 1 &&

                     monitor.snapshot ().alarmOutputInactive,

                 "first lifecycle starts qualifying and inactive");

        using Mutator           = void (*) (adk::MuseumCaseConfig&);

        const Mutator invalid[] = {
            [] (adk::MuseumCaseConfig& v)
            {
                v.ownerToken = 0;
            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.configurationRevision = 0;
            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.expectedLiquidSourceId = 0;
            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.expectedReedSourceId = v.expectedLiquidSourceId;
            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.expectedThermistorCalibrationRevision = 0;
            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.maximumLiquidAge = adk::Duration (0);

            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.maximumRadiantAge = adk::Duration (0x80000000UL);

            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.healthyCooldown = adk::Duration (0);

            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.auditReceiptDeadline = adk::Duration (0x80000000UL);

            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.maximumAuditAttempts = 0;
            },
            [] (adk::MuseumCaseConfig& v)
            {
                v.maximumAuditAttempts = 9;
            }};
        for (Mutator mutate : invalid)

        {
            adk::MuseumCaseConfig value = config ();

            mutate (value);

            adk::MuseumCaseMonitor rejected (value);

            require (rejected.initialize (adk::TimePoint (0)).error () ==

                             adk::StatusCode::InvalidConfiguration &&
                         !rejected.initialized (),

                     "invalid configuration remains inert");
        }
    }

    void testCoherenceAndAtomicRejection ()

    {
        adk::MuseumCaseMonitor monitor (config ());

        require (monitor.initialize (adk::TimePoint (90)).ok (),

                 "coherence monitor initializes");
        adk::MuseumCaseResult accepted = update (monitor, frame ());

        require (accepted.status.ok (), "baseline complete frame accepted");


        using Mutator           = void (*) (adk::MuseumCaseEnvelope&);

        const Mutator invalid[] = {
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.liquid.sample.sourceId = 9;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.environment.envelope.thermistor.calibrationRevision = 99;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.environment.envelope.digitalTemperature.sourceId = 2;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.environment.radiantQuality = static_cast<adk::RadiantQuality> (0xff);
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.reed.sequence = 0;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.reed.observation.rawLevel = adk::Level::Low;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.reed.observation.activationEvent = true;
                v.reed.observation.active          = false;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.acknowledge.configurationRevision = 99;
            },
            [] (adk::MuseumCaseEnvelope& v)
            {
                v.acknowledge.status = static_cast<adk::StatusCode> (0xff);
            }};
        for (Mutator mutate : invalid)

        {
            adk::MuseumCaseEnvelope candidate = frame (101, 2);

            mutate (candidate);

            adk::MuseumCaseResult       output = accepted;
            const adk::MuseumCaseIntent before = monitor.snapshot ();

            const adk::Status           status = monitor.update (candidate, output);

            require (status.error () == adk::StatusCode::InvalidArgument &&

                         output.status.error () == adk::StatusCode::InvalidArgument &&

                         !output.hasAuditIntent &&
                         intentEqual (output.intent, before) &&

                         intentEqual (before, monitor.snapshot ()),

                     "malformed envelope rejects atomically");
        }

        adk::MuseumCaseEnvelope     sameTime       = frame (100, 2);

        adk::MuseumCaseResult       output         = accepted;
        const adk::MuseumCaseIntent beforeSameTime = monitor.snapshot ();

        require (monitor.update (sameTime, output).error () ==

                         adk::StatusCode::InvalidArgument &&
                     !output.hasAuditIntent &&
                     intentEqual (output.intent, beforeSameTime) &&

                     intentEqual (beforeSameTime, monitor.snapshot ()),

                 "changed evidence at same policy time rejects atomically");

        adk::MuseumCaseEnvelope forgedProbe = frame (100, 1);

        forgedProbe.liquid.quality           = adk::ProbeQuality::Wet;
        const adk::MuseumCaseIntent beforeForgedProbe = monitor.snapshot ();

        require (monitor.update (forgedProbe, output).error () ==

                         adk::StatusCode::InvalidArgument &&
                     !output.hasAuditIntent &&
                     intentEqual (output.intent, beforeForgedProbe) &&
                     intentEqual (beforeForgedProbe, monitor.snapshot ()),

                 "same-time forged probe derivation rejects atomically");

        adk::MuseumCaseEnvelope forgedEnvironment = frame (100, 1);

        forgedEnvironment.environment.thermalQuality = adk::ThermalQuality::Alarm;
        forgedEnvironment.environment.thermalHazard  = true;
        const adk::MuseumCaseIntent beforeForgedEnvironment = monitor.snapshot ();

        require (monitor.update (forgedEnvironment, output).error () ==

                         adk::StatusCode::InvalidArgument &&
                     !output.hasAuditIntent &&
                     intentEqual (output.intent, beforeForgedEnvironment) &&
                     intentEqual (beforeForgedEnvironment, monitor.snapshot ()),

                 "same-time forged thermal derivation rejects atomically");

        adk::MuseumCaseMonitor receiptOnly (config ());

        require (receiptOnly.initialize (adk::TimePoint (90)).ok (),

                 "receipt-only same-time fixture initializes");
        const adk::MuseumCaseResult receiptFirst = update (receiptOnly, frame ());
        adk::MuseumCaseEnvelope receiptAtSameTime = frame ();

        receiptAtSameTime.hasAuditReceipt = true;
        receiptAtSameTime.auditReceipt    = receipt (receiptFirst.auditIntent);
        const adk::MuseumCaseResult receiptAccepted =
            update (receiptOnly, receiptAtSameTime);

        require (receiptAccepted.status.ok (),

                 "receipt-only update at the same policy time remains allowed");

        adk::MuseumCaseEnvelope changedDuplicate    = frame (101, 1);

        changedDuplicate.liquid.sample.energizedRaw = 700;
        require (monitor.update (changedDuplicate, output).error () ==

                     adk::StatusCode::InvalidArgument,
                 "same-sequence changed payload rejects");

        adk::MuseumCaseEnvelope regression = frame (101, 0xffffffffUL);

        require (monitor.update (regression, output).error () ==

                     adk::StatusCode::InvalidArgument,
                 "modular sequence regression rejects");
        adk::MuseumCaseEnvelope ambiguous = frame (101, 0x80000001UL);

        require (monitor.update (ambiguous, output).error () ==

                     adk::StatusCode::InvalidArgument,
                 "half-range sequence ambiguity rejects");
    }

    void testHazardsPrecedenceAndAlarmLatch ()

    {
        adk::MuseumCaseMonitor monitor (config ());

        require (monitor.initialize (adk::TimePoint (90)).ok (),

                 "hazard monitor initializes");
        adk::MuseumCaseEnvelope healthy = frame ();

        adk::MuseumCaseResult   first   = update (monitor, healthy);

        require (first.intent.health == adk::MuseumCaseHealth::Healthy &&

                     first.intent.hazardMask == 0 && first.hasAuditIntent,
                 "first complete healthy decision is audited");
        acceptOutstanding (monitor, healthy, first.auditIntent);


        adk::MuseumCaseEnvelope combined    = frame (101, 2);

        combined.liquid.quality             = adk::ProbeQuality::Wet;
        combined.environment.thermalQuality = adk::ThermalQuality::Alarm;
        combined.environment.thermalHazard  = true;
        combined.environment.radiantQuality = adk::RadiantQuality::AbruptChange;
        combined.environment.radiantHazard  = true;
        combined.reed.observation.active    = false;
        combined.reed.observation.raw       = 0;
        combined.reed.observation.rawLevel  = adk::Level::Low;
        adk::MuseumCaseResult alarm         = update (monitor, combined);

        const uint8_t         allHazards =
            hazard (adk::MuseumHazard::Liquid) | hazard (adk::MuseumHazard::Thermal) |

            hazard (adk::MuseumHazard::Radiant) | hazard (adk::MuseumHazard::Opening);

        require (alarm.intent.health == adk::MuseumCaseHealth::Alarm &&

                     alarm.intent.hazardMask == allHazards &&
                     alarm.intent.alarmSoundIntent &&
                     alarm.intent.inertRelayLampIntent &&
                     !alarm.intent.alarmOutputInactive,
                 "simultaneous hazards are retained and alarm outputs asserted");

        adk::MuseumCaseEnvelope fault = frame (102, 3);

        fault.liquid.quality          = adk::ProbeQuality::Disconnected;
        adk::MuseumCaseResult sensing = update (monitor, fault);

        require (sensing.intent.health == adk::MuseumCaseHealth::Fault &&

                     (sensing.intent.hazardMask &
                      hazard (adk::MuseumHazard::Sensing)) != 0 &&

                     sensing.intent.alarmOutputInactive,
                 "sensing fault dominates alarm and deactivates alarm outputs");

        adk::MuseumCaseEnvelope warning = frame (103, 4);

        warning.liquid.quality          = adk::ProbeQuality::Damp;
        adk::MuseumCaseResult warned    = update (monitor, warning);

        require (warned.intent.health == adk::MuseumCaseHealth::Warning,

                 "lesser hazard recovers fault to warning");

        adk::MuseumCaseEnvelope clear   = frame (104, 5);

        adk::MuseumCaseResult   latched = update (monitor, clear);

        require (latched.intent.health == adk::MuseumCaseHealth::Alarm &&

                     latched.intent.hazardMask == 0,
                 "warning recovery does not silently clear alarm latch");
    }

    void testAcknowledgementCooldown ()

    {
        adk::MuseumCaseConfig cooldownConfig = config ();

        cooldownConfig.auditReceiptDeadline  = adk::Duration (1000);

        adk::MuseumCaseMonitor monitor (cooldownConfig);

        require (monitor.initialize (adk::TimePoint (90)).ok (),

                 "cooldown monitor initializes");
        adk::MuseumCaseEnvelope alarmFrame = frame ();

        alarmFrame.liquid.quality          = adk::ProbeQuality::Wet;
        adk::MuseumCaseResult alarm        = update (monitor, alarmFrame);

        require (alarm.intent.health == adk::MuseumCaseHealth::Alarm,

                 "alarm fixture latches");

        adk::MuseumCaseEnvelope activeAck = frame (101, 2);

        activeAck.liquid.quality          = adk::ProbeQuality::Wet;
        activeAck.acknowledge.pressed     = true;
        require (update (monitor, activeAck).intent.health ==

                     adk::MuseumCaseHealth::Alarm,
                 "acknowledgement cannot clear active hazard");

        adk::MuseumCaseEnvelope cleared = frame (102, 3);

        require (update (monitor, cleared).intent.health ==

                     adk::MuseumCaseHealth::Alarm,
                 "cleared hazard remains latched without acknowledgement");

        adk::MuseumCaseEnvelope acknowledged = frame (103, 4);

        acknowledged.acknowledge.pressed     = true;
        require (update (monitor, acknowledged).intent.health ==

                     adk::MuseumCaseHealth::Cooldown,
                 "fresh acknowledgement begins cooldown");

        adk::MuseumCaseEnvelope edge = frame (123, 5);

        require (update (monitor, edge).intent.health == adk::MuseumCaseHealth::Healthy,

                 "inclusive cooldown boundary returns healthy");

        monitor.reset (adk::TimePoint (200));

        alarmFrame                            = frame (201, 1);

        alarmFrame.environment.thermalQuality = adk::ThermalQuality::Alarm;
        alarmFrame.environment.thermalHazard  = true;
        update (monitor, alarmFrame);

        acknowledged                     = frame (202, 2);

        acknowledged.acknowledge.pressed = true;
        update (monitor, acknowledged);

        adk::MuseumCaseEnvelope relapse    = frame (203, 3);

        relapse.environment.radiantQuality = adk::RadiantQuality::Sustained;
        relapse.environment.radiantHazard  = true;
        require (update (monitor, relapse).intent.health ==

                     adk::MuseumCaseHealth::Alarm,
                 "hazard during cooldown relatches alarm");
    }

    void testAuditDigestReceiptsAndDirtyCollision ()

    {
        const adk::MuseumAuditIntent canonical = {0x01020304UL,
                                                  0x11223344UL,
                                                  0x5566,
                                                  0x778899aaUL,
                                                  adk::TimePoint (0x0a0b0c0dUL),

                                                  adk::MuseumCaseHealth::Alarm,
                                                  0x19,
                                                  1,
                                                  2,
                                                  3,
                                                  4,
                                                  5,
                                                  6,
                                                  0,
                                                  adk::TimePoint (),

                                                  1};
        require (digest (canonical) == 0x4086e509UL,

                 "canonical witness vector freezes FNV field order");

        adk::MuseumCaseMonitor monitor (config ());

        require (monitor.initialize (adk::TimePoint (90)).ok (),

                 "audit monitor initializes");
        adk::MuseumCaseResult first = update (monitor, frame ());

        require (first.hasAuditIntent && first.auditIntent.attempt == 1 &&

                     first.auditIntent.recordSequence == 1,
                 "first sensed decision emits attempt one");
        require (first.auditIntent.witnessDigest == digest (first.auditIntent),

                 "audit digest is independent canonical FNV-1a");

        adk::MuseumCaseEnvelope repeat = frame (100, 1);

        repeat.now                     = adk::TimePoint (105);

        adk::MuseumCaseResult pending  = update (monitor, repeat);

        require (pending.hasAuditIntent &&

                     auditEqual (pending.auditIntent, first.auditIntent),

                 "same evidence repeats immutable outstanding intent");

        adk::MuseumCaseEnvelope dirty = frame (106, 2);

        dirty.liquid.quality          = adk::ProbeQuality::Wet;
        adk::MuseumCaseResult frozen  = update (monitor, dirty);

        require (frozen.hasAuditIntent &&

                     auditEqual (frozen.auditIntent, first.auditIntent) &&

                     frozen.intent.health == adk::MuseumCaseHealth::Alarm,
                 "decision change updates live intent but freezes outstanding witness");

        adk::MuseumCaseEnvelope wrong = frame (107, 3);

        wrong.liquid.quality          = adk::ProbeQuality::Wet;
        wrong.hasAuditReceipt         = true;
        wrong.auditReceipt            = receipt (first.auditIntent);

        ++wrong.auditReceipt.witnessDigest;
        adk::MuseumCaseResult       sentinel           = frozen;
        const adk::MuseumCaseIntent beforeWrongReceipt = monitor.snapshot ();

        require (monitor.update (wrong, sentinel).error () ==

                         adk::StatusCode::InvalidArgument &&
                     !sentinel.hasAuditIntent &&
                     intentEqual (sentinel.intent, beforeWrongReceipt) &&

                     intentEqual (beforeWrongReceipt, monitor.snapshot ()),

                 "wrong digest receipt rejects atomically");

        adk::MuseumCaseEnvelope collision    = frame (108, 4);

        collision.environment.thermalQuality = adk::ThermalQuality::Alarm;
        collision.environment.thermalHazard  = true;
        collision.reed.observation.active    = false;
        collision.reed.observation.raw       = 0;
        collision.reed.observation.rawLevel  = adk::Level::Low;
        collision.hasAuditReceipt            = true;
        collision.auditReceipt               = receipt (first.auditIntent);

        adk::MuseumCaseResult successor      = update (monitor, collision);

        require (successor.hasAuditIntent &&

                     successor.auditIntent.recordSequence == 2 &&
                     successor.auditIntent.attempt == 1 &&
                     successor.auditIntent.health == adk::MuseumCaseHealth::Alarm &&
                     (successor.auditIntent.hazardMask &
                      hazard (adk::MuseumHazard::Thermal)) != 0 &&

                     (successor.auditIntent.hazardMask &
                      hazard (adk::MuseumHazard::Opening)) != 0 &&

                     successor.auditIntent.liquidSequence == 4,
                 "accepted receipt and current decision promote latest successor");

        adk::MuseumCaseEnvelope duplicate    = collision;
        duplicate.now                        = adk::TimePoint (109);

        duplicate.hasAuditReceipt            = true;
        duplicate.auditReceipt               = receipt (first.auditIntent);

        adk::MuseumCaseResult afterDuplicate = update (monitor, duplicate);

        require (afterDuplicate.status.ok (),

                 "identical retired accepted receipt is idempotent");
    }

    void testRetryDeadlineAndTerminalReset ()

    {
        adk::MuseumCaseMonitor monitor (config (2));

        require (monitor.initialize (adk::TimePoint (0)).ok (),

                 "retry monitor initializes");
        adk::MuseumCaseResult first = update (monitor, frame (1, 1));


        adk::MuseumCaseEnvelope atDeadline = frame (1, 1);

        atDeadline.now                     = adk::TimePoint (11);

        adk::MuseumCaseResult inclusive    = update (monitor, atDeadline);

        require (inclusive.hasAuditIntent && inclusive.auditIntent.attempt == 1,

                 "inclusive receipt deadline remains pending");

        adk::MuseumCaseEnvelope lost = frame (1, 1);

        lost.now                     = adk::TimePoint (12);

        adk::MuseumCaseResult retry  = update (monitor, lost);

        require (

            !retry.hasAuditIntent &&
                retry.intent.health == adk::MuseumCaseHealth::Fault &&
                (retry.intent.hazardMask & hazard (adk::MuseumHazard::Recording)) != 0,

            "deadline loss records fault before retry issue");

        adk::MuseumCaseEnvelope guessed = frame (1, 1);

        guessed.now                     = adk::TimePoint (13);
        guessed.hasAuditReceipt         = true;
        guessed.auditReceipt            = receipt (first.auditIntent);
        guessed.auditReceipt.attempt    = 2;
        adk::MuseumCaseResult guessedResult = retry;
        const adk::MuseumCaseIntent beforeGuessedReceipt = monitor.snapshot ();

        require (monitor.update (guessed, guessedResult).error () ==

                         adk::StatusCode::InvalidArgument &&
                     !guessedResult.hasAuditIntent &&
                     intentEqual (guessedResult.intent, beforeGuessedReceipt) &&
                     intentEqual (beforeGuessedReceipt, monitor.snapshot ()),

                 "guessed retry receipt before reissue rejects atomically");

        adk::MuseumCaseEnvelope issue    = frame (1, 1);

        issue.now                        = adk::TimePoint (13);

        adk::MuseumCaseResult attemptTwo = update (monitor, issue);

        require (attemptTwo.hasAuditIntent && attemptTwo.auditIntent.attempt == 2 &&

                     attemptTwo.auditIntent.recordSequence ==
                         first.auditIntent.recordSequence &&
                     attemptTwo.auditIntent.witnessDigest ==
                         first.auditIntent.witnessDigest,
                 "retry reissues immutable witness once on later call");

        adk::MuseumCaseEnvelope failure = frame (1, 1);

        failure.now                     = adk::TimePoint (14);

        failure.hasAuditReceipt         = true;
        failure.auditReceipt            = receipt (attemptTwo.auditIntent, false);

        adk::MuseumCaseResult terminal  = update (monitor, failure);

        require (terminal.status.error () == adk::StatusCode::CapacityExceeded &&

                     terminal.intent.health == adk::MuseumCaseHealth::Fault &&
                     !terminal.hasAuditIntent,
                 "final failed attempt enters terminal recording fault");

        adk::MuseumCaseResult       rejected             = terminal;
        const adk::MuseumCaseIntent beforeTerminalReject = monitor.snapshot ();

        require (monitor.update (failure, rejected).error () ==

                         adk::StatusCode::InvalidArgument &&
                     !rejected.hasAuditIntent &&
                     intentEqual (rejected.intent, beforeTerminalReject) &&

                     intentEqual (beforeTerminalReject, monitor.snapshot ()),

                 "terminal audit state rejects further receipts atomically");
        require (monitor.reset (adk::TimePoint (20)).ok (),

                 "explicit reset clears terminal audit state");
        adk::MuseumCaseResult restarted = update (monitor, frame (21, 1));

        require (restarted.status.ok () && restarted.hasAuditIntent &&

                     restarted.auditIntent.lifecycleGeneration == 2,
                 "reset starts a fresh audited lifecycle");
    }

    void testRolloverExhaustionShutdownAndReplay ()

    {
        adk::MuseumCaseMonitor rollover (config ());

        require (rollover.initialize (adk::TimePoint (0xfffffff0UL)).ok (),

                 "rollover monitor initializes");
        adk::MuseumCaseResult wrapped = update (rollover, frame (0xfffffff8UL, 1));

        require (wrapped.status.ok (), "pre-wrap frame accepted");

        adk::MuseumCaseEnvelope after = frame (4, 2);

        require (update (rollover, after).status.ok (),

                 "policy time and evidence cross rollover");

        adk::MuseumCaseMonitor exhausted (config ());

        require (exhausted.initialize (adk::TimePoint (0)).ok (),

                 "exhaustion monitor initializes");
        adk::MuseumCaseEnvelope maximum = frame (1, 0xffffffffUL);

        require (update (exhausted, maximum).status.ok (),

                 "maximum producer sequence accepted once");
        adk::MuseumCaseEnvelope beyond   = frame (2, 1);

        adk::MuseumCaseResult   sentinel = update (exhausted, maximum);

        sentinel.status                  = adk::StatusCode::InternalInvariant;
        require (exhausted.update (beyond, sentinel).error () ==

                     adk::StatusCode::CapacityExceeded,
                 "producer sequence exhaustion fails closed");

        const uint32_t generation = rollover.snapshot ().lifecycleGeneration;

        require (rollover.shutdown ().ok () && !rollover.initialized () &&

                     rollover.snapshot ().lifecycleGeneration == generation + 1 &&

                     rollover.snapshot ().alarmOutputInactive &&

                     !rollover.snapshot ().alarmSoundIntent &&

                     !rollover.snapshot ().inertRelayLampIntent,

                 "shutdown invalidates lifecycle and all active intents");
        require (rollover.shutdown ().ok () &&

                     rollover.snapshot ().lifecycleGeneration == generation + 1,

                 "shutdown is idempotent");
        require (rollover.initialize (adk::TimePoint (10)).ok () &&

                     rollover.snapshot ().lifecycleGeneration == generation + 2,

                 "reinitialize advances lifecycle and requires fresh evidence");

        adk::MuseumCaseMonitor left (config ());

        adk::MuseumCaseMonitor right (config ());

        require (left.initialize (adk::TimePoint (0)).ok () &&

                     right.initialize (adk::TimePoint (0)).ok (),

                 "replay monitors initialize");
        for (uint32_t index = 1; index <= 8; ++index)

        {
            adk::MuseumCaseEnvelope input = frame (index, index);

            if (index == 2 || index == 3)

            {
                input.liquid.quality = adk::ProbeQuality::Wet;
            }
            if (index == 4)

            {
                input.environment.thermalQuality = adk::ThermalQuality::Disagreement;
            }
            const adk::MuseumCaseResult a = update (left, input);

            const adk::MuseumCaseResult b = update (right, input);

            require (resultEqual (a, b) &&

                         intentEqual (left.snapshot (), right.snapshot ()),

                     "identical envelopes produce field-identical replay");
        }
    }
} // namespace

int main ()

{
    testLifecycleAndConfiguration ();

    testCoherenceAndAtomicRejection ();

    testHazardsPrecedenceAndAlarmLatch ();

    testAcknowledgementCooldown ();

    testAuditDigestReceiptsAndDirtyCollision ();

    testRetryDeadlineAndTerminalReset ();

    testRolloverExhaustionShutdownAndReplay ();

}
// clang-format on
