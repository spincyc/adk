#include <interaction_intent_policy.h>

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

    adk::InteractionIntentConfig config ()
    {
        return {{adk::Level::High, adk::Duration (5), adk::Duration (3),
                 adk::Duration (20), adk::Duration (15)},
                adk::Duration (10),
                adk::Duration (10),
                300,
                200};
    }

    adk::InteractionSource contactSource (
        uint8_t id = 1, uint16_t revision = 1,
        adk::InteractionSourceKind kind = adk::InteractionSourceKind::CopiedContact)
    {
        return {kind, id, revision};
    }

    adk::InteractionSource directionalSource (
        uint8_t id = 2, uint16_t revision = 1,
        adk::InteractionSourceKind kind = adk::InteractionSourceKind::CopiedJoystick)
    {
        return {kind, id, revision};
    }

    adk::ContactSample contact (uint32_t observedAt, adk::Level level,
                                adk::Status status = adk::StatusCode::Ok)
    {
        return {adk::TimePoint (observedAt), level, status};
    }

    adk::DirectionalEvidence
    directional (uint32_t observedAt, uint32_t sequence, int16_t x = 0, int16_t y = 0,
                 bool saturated = false, adk::Status status = adk::StatusCode::Ok,
                 adk::InteractionSource source = directionalSource ())
    {
        return {source, adk::TimePoint (observedAt), sequence, x, y, saturated, status};
    }

    adk::Status update (adk::InteractionIntentPolicy& policy, uint32_t now,
                        const adk::InteractionSource& source, uint32_t contactSequence,
                        const adk::ContactSample&       contactSample,
                        const adk::DirectionalEvidence& directionalSample)
    {
        adk::InteractionIntentPreview candidate;
        const adk::Status             status =
            policy.preview (adk::TimePoint (now), source, contactSequence,
                            contactSample, directionalSample, candidate);
        if (!status.ok ())
        {
            return status;
        }
        require              (policy.canCommit (candidate), "successful preview is committable");
        return policy.commit (candidate);
    }

    bool sameSource (const adk::InteractionSource& left,
                     const adk::InteractionSource& right)
    {
        return left.kind == right.kind && left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision;
    }

    bool sameIntent (const adk::InteractionIntent& left,
                     const adk::InteractionIntent& right)
    {
        return sameSource (left.contactSource, right.contactSource) &&
               sameSource (left.directionalSource, right.directionalSource) &&
               left.observedAt == right.observedAt &&
               left.contactSequence == right.contactSequence &&
               left.directionalSequence == right.directionalSequence &&
               left.direction == right.direction &&
               left.magnitudePermille == right.magnitudePermille &&
               left.touchActive == right.touchActive &&
               left.touchEvent == right.touchEvent &&
               left.touchReleaseEvent == right.touchReleaseEvent &&
               left.directionEvent == right.directionEvent &&
               left.quality == right.quality && left.contactAge == right.contactAge &&
               left.directionalAge == right.directionalAge &&
               left.directionalSaturated == right.directionalSaturated &&
               left.contactQuality == right.contactQuality &&
               left.contactStatus == right.contactStatus &&
               left.directionalStatus == right.directionalStatus &&
               left.status == right.status;
    }

    void testLifecycleAndConfiguration ()
    {
        static_assert (!std::is_copy_constructible<adk::InteractionIntentPolicy>::value,
                       "interaction policy is not copy constructible");
        static_assert (!std::is_copy_assignable<adk::InteractionIntentPolicy>::value,
                       "interaction policy is not copy assignable");
        static_assert (!std::is_move_constructible<adk::InteractionIntentPolicy>::value,
                       "interaction policy is not move constructible");
        static_assert (!std::is_move_assignable<adk::InteractionIntentPolicy>::value,
                       "interaction policy is not move assignable");

        adk::InteractionIntentPolicy  policy (config ());
        adk::InteractionIntentPreview candidate;
        require (!policy.initialized (), "construction is inert");
        require (policy.snapshot ().status.error () == adk::StatusCode::NotInitialized,
                 "construction snapshot is canonical");
        require (policy.preview (adk::TimePoint (), contactSource (), 1,
                                 contact (0, adk::Level::Low), directional (0, 1),
                                 candidate)
                         .error () == adk::StatusCode::NotInitialized,
                 "preview before initialize is rejected");
        require (!policy.canCommit (candidate), "default preview cannot commit");
        require (policy.commit (candidate).error () == adk::StatusCode::InvalidArgument,
                 "default preview commit is rejected");
        require (policy.initialize ().ok () && policy.initialize ().ok (),
                 "initialize is idempotent");
        require (policy.initialized (), "initialize activates policy");
        require (policy.snapshot ().quality == adk::InteractionQuality::Invalid &&
                     policy.snapshot ().status.ok (),
                 "initialize manufactures no observation");

        const adk::Duration                valid (1);
        const adk::Duration                zero;
        const adk::Duration                half        (0x80000000UL);
        const adk::ContactDynamicsConfig   goodContact (adk::Level::High, valid, valid,
                                                        valid, valid);
        const adk::InteractionIntentConfig invalid[] = {
            {goodContact, zero, valid, 1, 0},
            {goodContact, half, valid, 1, 0},
            {goodContact, valid, zero, 1, 0},
            {goodContact, valid, half, 1, 0},
            {goodContact, valid, valid, 0, 0},
            {goodContact, valid, valid, 1001, 0},
            {goodContact, valid, valid, 500, 501},
            {{static_cast<adk::Level> (2), valid, valid, valid, valid},
             valid,
             valid,
             1,
             0},
            {{adk::Level::High, zero, valid, valid, valid}, valid, valid, 1, 0},
            {{adk::Level::High, valid, half, valid, valid}, valid, valid, 1, 0}};
        for (const auto& invalidConfig : invalid)
        {
            adk::InteractionIntentPolicy rejected (invalidConfig);
            require                               (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid configuration is rejected");
            require (!rejected.initialized (), "invalid policy remains inert");
            require (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid initialization retry is deterministic");
        }

        policy.reset ();
        require      (policy.initialized () &&
                     policy.snapshot  ().quality == adk::InteractionQuality::Invalid &&
                     !policy.snapshot ().touchEvent &&
                     !policy.snapshot ().directionEvent,
                 "reset returns initialized event-free baseline");
    }

    void testStructuralValidationAndAtomicity ()
    {
        adk::InteractionIntentPolicy policy (config ());
        require                             (policy.initialize ().ok (), "validation fixture initializes");
        require                             (update (policy, 1, contactSource (), 1, contact (1, adk::Level::Low),
                         directional (1, 1))
                     .ok (),
                 "baseline commits");
        const adk::InteractionIntent baseline = policy.snapshot ();

        const adk::InteractionSource invalidContact[] = {
            contactSource (0), contactSource (1, 0),
            contactSource (1, 1, adk::InteractionSourceKind::CopiedJoystick),
            contactSource (1, 1, static_cast<adk::InteractionSourceKind> (3))};
        for (const auto& source : invalidContact)
        {
            adk::InteractionIntentPreview candidate;
            require (policy.preview (adk::TimePoint (2), source, 2,
                                     contact (2, adk::Level::Low), directional (2, 2),
                                     candidate)
                             .error () == adk::StatusCode::InvalidArgument,
                     "invalid contact source is rejected");
            require (sameIntent (policy.snapshot (), baseline),
                     "invalid contact source is atomic");
        }

        const adk::InteractionSource invalidDirectional[] = {
            directionalSource (0), directionalSource (1, 0),
            directionalSource (1, 1, adk::InteractionSourceKind::CopiedContact),
            directionalSource (1, 1, static_cast<adk::InteractionSourceKind> (3))};
        for (const auto& source : invalidDirectional)
        {
            adk::InteractionIntentPreview candidate;
            require (policy.preview (adk::TimePoint (2), contactSource (), 2,
                                     contact     (2, adk::Level::Low),
                                     directional (2, 2, 0, 0, false,
                                                  adk::StatusCode::Ok, source),
                                     candidate)
                             .error () == adk::StatusCode::InvalidArgument,
                     "invalid directional source is rejected");
            require (sameIntent (policy.snapshot (), baseline),
                     "invalid directional source is atomic");
        }

        const int16_t invalidAxes[][2] = {{-1001, 0},
                                          {1001, 0},
                                          {0, -1001},
                                          {0, 1001},
                                          {std::numeric_limits<int16_t>::min (), 0}};
        for (const auto& axes : invalidAxes)
        {
            adk::InteractionIntentPreview candidate;
            require (policy.preview (adk::TimePoint (2), contactSource (), 2,
                                     contact     (2, adk::Level::Low),
                                     directional (2, 2, axes[0], axes[1]), candidate)
                             .error () == adk::StatusCode::InvalidArgument,
                     "out-of-range axis is rejected safely");
        }

        adk::InteractionIntentPreview candidate;
        require (policy.preview (adk::TimePoint (2), contactSource (), 2,
                                 contact     (2, static_cast<adk::Level> (2)),
                                 directional (2, 2), candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "unknown contact level is rejected");
        require (policy.preview (adk::TimePoint (2), contactSource (), 2,
                                 contact (2, adk::Level::Low,
                                          static_cast<adk::StatusCode> (11)),
                                 directional (2, 2), candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "unknown contact status is rejected");
        require (policy.preview (adk::TimePoint (2), contactSource (), 2,
                                 contact     (2, adk::Level::Low),
                                 directional (2, 2, 0, 0, false,
                                              static_cast<adk::StatusCode> (11)),
                                 candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "unknown directional status is rejected");
        require (policy.preview (adk::TimePoint (1), contactSource (), 2,
                                 contact (2, adk::Level::Low), directional (1, 2),
                                 candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "future contact observation is rejected");
        require (policy.preview (adk::TimePoint (1), contactSource (), 2,
                                 contact (1, adk::Level::Low), directional (2, 2),
                                 candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "future directional observation is rejected");
        require (sameIntent (policy.snapshot (), baseline),
                 "all structural failures preserve snapshot");
    }

    void testDirectionOctantsHysteresisAndSaturation ()
    {
        struct DirectionCase
        {
            int16_t                   x;
            int16_t                   y;
            adk::InteractionDirection expected;
        };
        const DirectionCase cases[] = {
            {300, 0, adk::InteractionDirection::East},
            {-300, 0, adk::InteractionDirection::West},
            {0, 300, adk::InteractionDirection::North},
            {0, -300, adk::InteractionDirection::South},
            {300, 300, adk::InteractionDirection::NorthEast},
            {300, -300, adk::InteractionDirection::SouthEast},
            {-300, -300, adk::InteractionDirection::SouthWest},
            {-300, 300, adk::InteractionDirection::NorthWest},
            {1000, 414, adk::InteractionDirection::East},
            {1000, 415, adk::InteractionDirection::NorthEast},
            {-1000, 414, adk::InteractionDirection::West},
            {-1000, 415, adk::InteractionDirection::NorthWest},
            {414, -1000, adk::InteractionDirection::South},
            {415, -1000, adk::InteractionDirection::SouthEast}};

        for (const auto& item : cases)
        {
            adk::InteractionIntentPolicy policy (config ());
            require                             (policy.initialize ().ok (), "octant fixture initializes");
            require                             (update (policy, 0, contactSource (), 1,
                             contact (0, adk::Level::Low), directional (0, 1, 0, 0))
                         .ok (),
                     "octant baseline commits");
            require (update (policy, 1, contactSource (), 2,
                             contact     (1, adk::Level::Low),
                             directional (1, 2, item.x, item.y))
                         .ok (),
                     "octant sample commits");
            require (policy.snapshot ().direction == item.expected &&
                         policy.snapshot ().directionEvent,
                     "octant and exact boundary classify deterministically");
        }

        adk::InteractionIntentPolicy policy (config ());
        require                             (policy.initialize ().ok (), "hysteresis fixture initializes");
        update                              (policy, 0, contactSource (), 1, contact (0, adk::Level::Low),
                directional (0, 1, 299, 0));
        require (policy.snapshot ().direction == adk::InteractionDirection::Neutral,
                 "one below engage remains neutral");
        update (policy, 1, contactSource (), 2, contact (1, adk::Level::Low),
                directional (1, 2, 300, 0));
        require (policy.snapshot ().direction == adk::InteractionDirection::East &&
                     policy.snapshot ().directionEvent,
                 "exact engage edge emits event");
        update (policy, 2, contactSource (), 3, contact (2, adk::Level::Low),
                directional (2, 3, 200, 0));
        require (policy.snapshot ().direction == adk::InteractionDirection::East &&
                     !policy.snapshot ().directionEvent,
                 "exact release edge remains engaged");
        update (policy, 3, contactSource (), 4, contact (3, adk::Level::Low),
                directional (3, 4, 199, 0));
        require (policy.snapshot ().direction == adk::InteractionDirection::Neutral &&
                     policy.snapshot ().directionEvent,
                 "one below release returns neutral with event");
        update (policy, 4, contactSource (), 5, contact (4, adk::Level::Low),
                directional (4, 5, 1000, -1000, true));
        require (policy.snapshot ().direction == adk::InteractionDirection::SouthEast &&
                     policy.snapshot ().magnitudePermille == 1000 &&
                     policy.snapshot ().directionalSaturated &&
                     policy.snapshot ().quality == adk::InteractionQuality::Current,
                 "saturation is retained valid extreme evidence");
    }

    void testContactReuseAndQualityPrecedence ()
    {
        adk::InteractionIntentPolicy policy (config ());
        require                             (policy.initialize ().ok (), "contact fixture initializes");
        update                              (policy, 0, contactSource (), 1, contact (0, adk::Level::Low),
                directional (0, 1));
        update (policy, 1, contactSource (), 2, contact (1, adk::Level::High),
                directional (1, 2));
        update (policy, 5, contactSource (), 3, contact (5, adk::Level::High),
                directional (5, 3));
        require (!policy.snapshot ().touchEvent, "one tick before qualify");
        update  (policy, 6, contactSource (), 4, contact (6, adk::Level::High),
                directional (6, 4, 300, 0));
        require (policy.snapshot ().touchActive && policy.snapshot ().touchEvent &&
                     policy.snapshot ().directionEvent,
                 "simultaneous contact and direction events publish coherently");
        update (policy, 7, contactSource (), 5, contact (7, adk::Level::Low),
                directional (7, 5, 300, 0));
        update (policy, 9, contactSource (), 6, contact (9, adk::Level::Low),
                directional (9, 6, 300, 0));
        require (!policy.snapshot ().touchReleaseEvent,
                 "one tick before release remains active");
        update (policy, 10, contactSource (), 7, contact (10, adk::Level::Low),
                directional (10, 7, 300, 0));
        require (policy.snapshot ().touchReleaseEvent &&
                     !policy.snapshot ().touchActive,
                 "exact release edge publishes");

        policy.reset ();
        update       (policy, 20, contactSource (), 1, contact (20, adk::Level::High),
                directional (20, 1));
        update (policy, 25, contactSource (), 2, contact (25, adk::Level::High),
                directional (25, 2));
        update (policy, 39, contactSource (), 3, contact (39, adk::Level::High),
                directional (39, 3));
        require (policy.snapshot ().quality == adk::InteractionQuality::Current,
                 "one tick before stuck is current");
        update (policy, 40, contactSource (), 4, contact (40, adk::Level::High),
                directional (40, 4));
        require (policy.snapshot ().quality == adk::InteractionQuality::StuckActive &&
                     policy.snapshot ().touchActive,
                 "stuck-active classification retains active state");

        policy.reset ();
        update       (policy, 50, contactSource (), 1,
                contact     (50, adk::Level::Low, adk::StatusCode::HardwareFailure),
                directional (50, 1, 500, 0));
        require (policy.snapshot ().quality == adk::InteractionQuality::SourceFault &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::HardwareFailure &&
                     !policy.snapshot ().touchEvent &&
                     !policy.snapshot ().directionEvent &&
                     policy.snapshot  ().direction == adk::InteractionDirection::Neutral,
                 "contact source fault suppresses usable intent");

        policy.reset ();
        update       (policy, 60, contactSource (), 1, contact (60, adk::Level::Low),
                directional (60, 1, 500, 0, false, adk::StatusCode::Unsupported));
        require (policy.snapshot ().quality == adk::InteractionQuality::SourceFault &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::Unsupported &&
                     policy.snapshot ().direction == adk::InteractionDirection::Neutral,
                 "directional source fault is not hidden");
    }

    void testTimeSequenceProvenanceAndRepeats ()
    {
        adk::InteractionIntentPolicy policy (config ());
        require                             (policy.initialize ().ok (), "sequence fixture initializes");
        update                              (policy, 100, contactSource (), 0xfffffffeUL,
                contact (100, adk::Level::Low), directional (100, 0xfffffffeUL));
        update (policy, 101, contactSource (), 0xffffffffUL,
                contact     (101, adk::Level::Low),
                directional (101, 0xffffffffUL, 300, 0));
        update (policy, 102, contactSource (), 0, contact (102, adk::Level::Low),
                directional (102, 0, 400, 0));
        require (policy.snapshot ().contactSequence == 0 &&
                     policy.snapshot ().directionalSequence == 0,
                 "sequence wrap advances");

        const adk::InteractionIntent  beforeInvalid = policy.snapshot ();
        adk::InteractionIntentPreview candidate;
        require (policy.preview (adk::TimePoint (103), contactSource (), 0x80000000UL,
                                 contact (103, adk::Level::Low), directional (103, 1),
                                 candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "ambiguous contact half-range is rejected");
        require (policy.preview (adk::TimePoint (103), contactSource (), 1,
                                 contact     (103, adk::Level::Low),
                                 directional (103, 0x80000000UL), candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "ambiguous directional half-range is rejected");
        require (policy.preview (adk::TimePoint (103), contactSource (), 0xffffffffUL,
                                 contact (103, adk::Level::Low), directional (103, 1),
                                 candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "contact regression is rejected");
        require (sameIntent (policy.snapshot (), beforeInvalid),
                 "sequence failures are atomic");

        policy.reset                                                ();
        const adk::ContactSample       contactFrame   = contact     (10, adk::Level::Low);
        const adk::DirectionalEvidence directionFrame = directional (10, 10, 300, 0);
        update                                                      (policy, 10, contactSource (), 10, contactFrame, directionFrame);
        require                                                     (update (policy, 20, contactSource (), 10, contactFrame, directionFrame)
                     .ok (),
                 "exact repeats may age");
        require (!policy.snapshot ().touchEvent && !policy.snapshot ().directionEvent &&
                     policy.snapshot ().contactAge == adk::Duration (10) &&
                     policy.snapshot ().directionalAge == adk::Duration (10) &&
                     policy.snapshot ().quality == adk::InteractionQuality::Current,
                 "repeat clears events at exact age boundary");
        update  (policy, 21, contactSource (), 10, contactFrame, directionFrame);
        require (policy.snapshot ().quality == adk::InteractionQuality::Stale,
                 "repeat becomes stale after age boundary");

        policy.reset ();
        update       (policy, 10, contactSource (), 10, contactFrame, directionFrame);
        require      (policy.preview (adk::TimePoint (11), contactSource (), 10,
                                 contact (11, adk::Level::Low), directionFrame,
                                 candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "changed contact payload at same sequence is rejected");
        require (policy.preview (adk::TimePoint (11), contactSource (), 11,
                                 contact     (11, adk::Level::Low),
                                 directional (11, 10, 301, 0), candidate)
                         .error () == adk::StatusCode::InvalidArgument,
                 "changed directional payload at same sequence is rejected");

        require (update (policy, 11, contactSource (3, 2), 1,
                         contact     (11, adk::Level::Low),
                         directional (11, 1, -500, 0, false, adk::StatusCode::Ok,
                                      directionalSource (4, 2)))
                     .ok (),
                 "source-domain change accepts fresh sequences");
        require (
            policy.snapshot ().direction == adk::InteractionDirection::West &&
                !policy.snapshot ().directionEvent &&
                sameSource       (policy.snapshot ().contactSource, contactSource (3, 2)) &&
                sameSource       (policy.snapshot ().directionalSource,
                            directionalSource (4, 2)),
            "source change publishes provenance without fabricated event");

        policy.reset ();
        const uint32_t nearWrap = 0xfffffff8UL;
        update (policy, 2, contactSource (), 1, contact (nearWrap, adk::Level::Low),
                directional (nearWrap, 1));
        require (policy.snapshot ().contactAge == adk::Duration (10) &&
                     policy.snapshot ().directionalAge == adk::Duration (10),
                 "TimePoint rollover computes wrap-safe age");
    }

    void testRecoveryPolarityRefractoryAndTimeFaults ()
    {
        adk::InteractionIntentConfig activeLowConfig = config ();
        activeLowConfig.contact.activeLevel          = adk::Level::Low;
        adk::InteractionIntentPolicy activeLow (activeLowConfig);
        require                                (activeLow.initialize ().ok (),
                 "active-low fixture initializes");
        update (activeLow, 0, contactSource (), 1, contact (0, adk::Level::High),
                directional (0, 1));
        update (activeLow, 1, contactSource (), 2, contact (1, adk::Level::Low),
                directional (1, 2));
        update (activeLow, 6, contactSource (), 3, contact (6, adk::Level::Low),
                directional (6, 3));
        require (activeLow.snapshot ().touchEvent && activeLow.snapshot ().touchActive,
                 "active-low qualification reuses contact policy");

        adk::InteractionIntentPolicy refractory (config ());
        require                                 (refractory.initialize ().ok (), "refractory fixture initializes");
        update                                  (refractory, 0, contactSource (), 1, contact (0, adk::Level::High),
                directional (0, 1));
        update (refractory, 5, contactSource (), 2, contact (5, adk::Level::High),
                directional (5, 2));
        require (refractory.snapshot ().touchEvent,
                 "first qualified contact is accepted");
        update (refractory, 6, contactSource (), 3, contact (6, adk::Level::Low),
                directional (6, 3));
        update (refractory, 9, contactSource (), 4, contact (9, adk::Level::Low),
                directional (9, 4));
        update (refractory, 10, contactSource (), 5, contact (10, adk::Level::High),
                directional (10, 5));
        update (refractory, 15, contactSource (), 6, contact (15, adk::Level::High),
                directional (15, 6));
        require (!refractory.snapshot ().touchEvent &&
                     refractory.snapshot ().touchActive,
                 "qualified attack inside refractory is suppressed");
        update (refractory, 16, contactSource (), 7, contact (16, adk::Level::Low),
                directional (16, 7));
        update (refractory, 19, contactSource (), 8, contact (19, adk::Level::Low),
                directional (19, 8));
        update (refractory, 25, contactSource (), 9, contact (25, adk::Level::High),
                directional (25, 9));
        update (refractory, 30, contactSource (), 10,
                contact (30, adk::Level::High), directional (30, 10));
        require (refractory.snapshot ().touchEvent,
                 "attack at completed refractory edge is accepted");

        adk::InteractionIntentPolicy recovery (config ());
        require                               (recovery.initialize ().ok (), "recovery fixture initializes");
        update                                (recovery, 0, contactSource (), 1,
                contact     (0, adk::Level::Low, adk::StatusCode::HardwareFailure),
                directional (0, 1, 500, 0));
        update (recovery, 1, contactSource (), 2, contact (1, adk::Level::Low),
                directional (1, 2, 500, 0));
        require (recovery.snapshot ().quality == adk::InteractionQuality::Current &&
                     !recovery.snapshot ().touchEvent &&
                     !recovery.snapshot ().directionEvent,
                 "fresh evidence resets contact fault without fabricated events");
        update (recovery, 2, contactSource (), 3, contact (2, adk::Level::Low),
                directional (2, 3, 500, 0, false,
                             adk::StatusCode::HardwareFailure));
        update (recovery, 3, contactSource (), 4, contact (3, adk::Level::Low),
                directional (3, 4, -500, 0));
        require (recovery.snapshot ().quality == adk::InteractionQuality::Current &&
                     recovery.snapshot  ().direction == adk::InteractionDirection::West &&
                     !recovery.snapshot ().directionEvent,
                 "fresh evidence resets directional fault as a new baseline");
        update (recovery, 4, contactSource (), 5,
                contact     (4, adk::Level::Low, adk::StatusCode::HardwareFailure),
                directional (4, 5, 500, 0, false, adk::StatusCode::Unsupported));
        require (recovery.snapshot ().quality ==
                         adk::InteractionQuality::SourceFault &&
                     recovery.snapshot ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                 "contact status wins colliding source faults");
        recovery.reset ();
        update         (recovery, 5, contactSource (), 1, contact (5, adk::Level::Low),
                directional (5, 1));
        require (recovery.snapshot ().quality == adk::InteractionQuality::Current,
                 "reset recovers a faulted policy");

        const adk::InteractionIntent baseline = recovery.snapshot ();
        adk::InteractionIntentPreview candidate;
        require (recovery.preview (adk::TimePoint (4), contactSource (), 2,
                                   contact (4, adk::Level::Low), directional (4, 2),
                                   candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "frame-time regression is rejected");
        require (recovery.preview (adk::TimePoint (0x80000005UL), contactSource (), 2,
                                   contact (6, adk::Level::Low), directional (6, 2),
                                   candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "frame-time half-range is rejected");
        require (recovery.preview (adk::TimePoint (7), contactSource (), 2,
                                   contact (4, adk::Level::Low), directional (7, 2),
                                   candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "contact observation-time regression is rejected");
        require (recovery.preview (adk::TimePoint (7), contactSource (), 2,
                                   contact (7, adk::Level::Low), directional (4, 2),
                                   candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "directional observation-time regression is rejected");
        require (sameIntent (recovery.snapshot (), baseline),
                 "time failures preserve committed state");

        adk::InteractionIntentConfig zeroRelease = config ();
        zeroRelease.releaseMagnitudePermille     = 0;
        adk::InteractionIntentPolicy zeroPolicy (zeroRelease);
        require                                 (zeroPolicy.initialize ().ok (), "zero-release fixture initializes");
        update                                  (zeroPolicy, 0, contactSource (), 1, contact (0, adk::Level::Low),
                directional (0, 1, 300, 0));
        update (zeroPolicy, 1, contactSource (), 2, contact (1, adk::Level::Low),
                directional (1, 2, 0, 0));
        require (zeroPolicy.snapshot ().direction ==
                         adk::InteractionDirection::Neutral &&
                     zeroPolicy.snapshot ().directionEvent,
                 "zero vector releases when release threshold is zero");
    }

    void testOpaqueTransactionsAndReplay ()
    {
        adk::InteractionIntentPolicy left  (config ());
        adk::InteractionIntentPolicy right (config ());
        require                            (left.initialize ().ok () && right.initialize ().ok (),
                 "transaction fixtures initialize");

        adk::InteractionIntentPreview foreign;
        require (left.preview (adk::TimePoint (0), contactSource (), 1,
                               contact (0, adk::Level::Low), directional (0, 1),
                               foreign)
                     .ok (),
                 "foreign candidate previews");
        require (!right.canCommit (foreign) && right.commit (foreign).error () ==
                                                   adk::StatusCode::InvalidArgument,
                 "owner binding rejects foreign candidate");

        adk::InteractionIntentPreview stale;
        require (left.preview (adk::TimePoint (1), contactSource (), 2,
                               contact (1, adk::Level::Low), directional (1, 2), stale)
                     .ok (),
                 "stale candidate previews");
        require (left.commit (foreign).ok (), "first candidate commits");
        require (!left.canCommit (stale) &&
                     left.commit (stale).error () == adk::StatusCode::InvalidArgument,
                 "generation binding rejects stale candidate");
        require (!left.canCommit (foreign) &&
                     left.commit (foreign).error () == adk::StatusCode::InvalidArgument,
                 "candidate cannot be reused");

        adk::InteractionIntentPreview resetCandidate;
        require (left.preview (adk::TimePoint (2), contactSource (), 2,
                               contact (2, adk::Level::Low), directional (2, 2),
                               resetCandidate)
                     .ok (),
                 "reset candidate previews");
        left.reset ();
        require    (!left.canCommit (resetCandidate),
                 "reset invalidates outstanding candidate");

        struct Frame
        {
            uint32_t   now;
            uint32_t   sequence;
            adk::Level level;
            int16_t    x;
            int16_t    y;
        };
        const Frame frames[] = {{10, 1, adk::Level::Low, 0, 0},
                                {11, 2, adk::Level::High, 0, 0},
                                {16, 3, adk::Level::High, 500, 500},
                                {17, 4, adk::Level::Low, 500, 0},
                                {20, 5, adk::Level::Low, 100, 0}};

        adk::InteractionIntentPolicy replayA (config ());
        adk::InteractionIntentPolicy replayB (config ());
        require                              (replayA.initialize ().ok () && replayB.initialize ().ok (),
                 "replay policies initialize");
        for (const auto& frame : frames)
        {
            require (update (replayA, frame.now, contactSource (), frame.sequence,
                             contact     (frame.now, frame.level),
                             directional (frame.now, frame.sequence, frame.x, frame.y))
                         .ok (),
                     "first replay frame commits");
            require (update (replayB, frame.now, contactSource (), frame.sequence,
                             contact     (frame.now, frame.level),
                             directional (frame.now, frame.sequence, frame.x, frame.y))
                         .ok (),
                     "second replay frame commits");
            require (sameIntent (replayA.snapshot (), replayB.snapshot ()),
                     "replay output fields are byte-value identical");
        }
    }

    void testOracleStatusesAndRemainingBoundaries ()
    {
        adk::InteractionIntentPolicy facade (config ());
        adk::ContactDynamics         oracle (config ().contact);
        require                             (facade.initialize ().ok () && oracle.initialize ().ok (),
                 "oracle fixtures initialize");
        struct ContactFrame
        {
            uint32_t   at;
            adk::Level level;
        };
        const ContactFrame frames[] = {
            {0, adk::Level::Low},   {1, adk::Level::High},
            {4, adk::Level::Low},   {5, adk::Level::High},
            {9, adk::Level::High},  {10, adk::Level::High},
            {11, adk::Level::Low},  {13, adk::Level::Low},
            {14, adk::Level::Low},  {30, adk::Level::High},
            {35, adk::Level::High}, {49, adk::Level::High},
            {50, adk::Level::High}};
        uint32_t sequence = 1;
        for (const auto& frame : frames)
        {
            const adk::ContactSample sample = contact (frame.at, frame.level);
            require                                   (oracle.update (sample).ok (), "direct contact oracle updates");
            require                                   (update (facade, frame.at, contactSource (), sequence, sample,
                             directional (frame.at, sequence))
                         .ok (),
                     "facade oracle frame commits");
            const adk::ContactObservation expected = oracle.snapshot ();
            const adk::InteractionIntent  actual   = facade.snapshot ();
            require                                                  (actual.contactQuality == expected.quality &&
                         actual.contactStatus == expected.status &&
                         actual.touchActive == expected.qualifiedActive &&
                         actual.touchEvent ==
                             (expected.attackEvent &&
                              expected.disposition ==
                                  adk::ContactDisposition::Accepted) &&
                         actual.touchReleaseEvent == expected.releaseEvent,
                     "facade matches direct ContactDynamics oracle");
            ++sequence;
        }
        require (oracle.snapshot ().acceptedCount == 2 &&
                     oracle.snapshot ().suppressedCount == 0 &&
                     oracle.snapshot ().quality == adk::ContactQuality::StuckActive,
                 "oracle proves qualification release stuck and counters");

        adk::InteractionIntentPolicy sequences (config ());
        require                                (sequences.initialize ().ok (), "remaining sequence fixture initializes");
        update                                 (sequences, 0, contactSource (), 1, contact (0, adk::Level::Low),
                directional (0, 1));
        require (update (sequences, 1, contactSource (), 9,
                         contact (1, adk::Level::Low), directional (1, 17, 300, 0))
                     .ok (),
                 "forward gaps in both streams are accepted");
        const adk::InteractionIntent gap = sequences.snapshot ();
        adk::InteractionIntentPreview candidate;
        require (sequences.preview (adk::TimePoint (2), contactSource (), 10,
                                    contact     (2, adk::Level::Low),
                                    directional (2, 16, 300, 0), candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "ordinary directional regression is rejected");
        require (sequences.preview (adk::TimePoint (2), contactSource (), 10,
                                    contact     (1, adk::Level::High),
                                    directional (2, 18, 300, 0), candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "changed contact at same observation time is rejected");
        require (sequences.preview (adk::TimePoint (2), contactSource (), 10,
                                    contact     (2, adk::Level::Low),
                                    directional (1, 18, 301, 0), candidate)
                     .error () == adk::StatusCode::InvalidArgument,
                 "changed directional evidence at same time is rejected");
        require (sameIntent (sequences.snapshot (), gap),
                 "remaining sequence failures preserve state");

        const adk::StatusCode statuses[] = {
            adk::StatusCode::Ok,
            adk::StatusCode::InvalidArgument,
            adk::StatusCode::InvalidConfiguration,
            adk::StatusCode::InvalidPin,
            adk::StatusCode::Unsupported,
            adk::StatusCode::ResourceBusy,
            adk::StatusCode::NotInitialized,
            adk::StatusCode::CapacityExceeded,
            adk::StatusCode::Timeout,
            adk::StatusCode::InternalInvariant,
            adk::StatusCode::HardwareFailure};
        for (const adk::StatusCode status : statuses)
        {
            adk::InteractionIntentPolicy contactStatusPolicy (config ());
            require                                          (contactStatusPolicy.initialize ().ok (),
                     "contact status fixture initializes");
            require (update (contactStatusPolicy, 0, contactSource (), 1,
                             contact     (0, adk::Level::Low, status),
                             directional (0, 1))
                         .error () == status,
                     "all declared contact statuses propagate exactly");

            adk::InteractionIntentPolicy directionStatusPolicy (config ());
            require                                            (directionStatusPolicy.initialize ().ok (),
                     "direction status fixture initializes");
            require (update (directionStatusPolicy, 0, contactSource (), 1,
                             contact     (0, adk::Level::Low),
                             directional (0, 1, 0, 0, false, status))
                         .error () == status,
                     "all declared directional statuses propagate exactly");
        }

        adk::InteractionIntentPolicy staleRecovery (config ());
        require                                    (staleRecovery.initialize ().ok (),
                 "stale recovery fixture initializes");
        const adk::ContactSample staleContact = contact (0, adk::Level::Low);
        const adk::DirectionalEvidence staleDirection =
            directional (0, 1, 500, 0);
        update  (staleRecovery, 0, contactSource (), 1, staleContact, staleDirection);
        update  (staleRecovery, 11, contactSource (), 1, staleContact, staleDirection);
        require (staleRecovery.snapshot ().quality == adk::InteractionQuality::Stale,
                 "repeated evidence becomes stale");
        update (staleRecovery, 12, contactSource (), 2,
                contact (12, adk::Level::Low), directional (12, 2, 500, 0));
        require (staleRecovery.snapshot ().quality ==
                         adk::InteractionQuality::Current &&
                     staleRecovery.snapshot ().direction ==
                         adk::InteractionDirection::East &&
                     !staleRecovery.snapshot ().directionEvent,
                 "fresh evidence after stale establishes event-free baseline");

        adk::InteractionIntentPolicy rollover (config ());
        require                               (rollover.initialize ().ok (), "rollover fixture initializes");
        update                                (rollover, 0xfffffff0UL, contactSource (), 0xfffffffeUL,
                contact     (0xfffffff0UL, adk::Level::Low),
                directional (0xfffffff0UL, 0xfffffffeUL));
        update (rollover, 0xffffffffUL, contactSource (), 0xffffffffUL,
                contact     (0xffffffffUL, adk::Level::Low),
                directional (0xffffffffUL, 0xffffffffUL, 300, 0));
        update (rollover, 0, contactSource (), 0,
                contact (0, adk::Level::Low), directional (0, 0, 300, 300));
        require (rollover.snapshot ().observedAt == adk::TimePoint (0) &&
                     rollover.snapshot ().contactSequence == 0 &&
                     rollover.snapshot ().directionalSequence == 0 &&
                     rollover.snapshot ().direction ==
                         adk::InteractionDirection::NorthEast,
                 "frame time and both sequences progress across rollover");
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration               ();
    testStructuralValidationAndAtomicity        ();
    testDirectionOctantsHysteresisAndSaturation ();
    testContactReuseAndQualityPrecedence        ();
    testTimeSequenceProvenanceAndRepeats        ();
    testRecoveryPolarityRefractoryAndTimeFaults ();
    testOpaqueTransactionsAndReplay             ();
    testOracleStatusesAndRemainingBoundaries    ();
    std::cout << "interaction intent policy tests passed\n";
    return EXIT_SUCCESS;
}
// clang-format on
