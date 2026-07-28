#define private public
#include <contact_dynamics.h>
#undef private

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

    adk::ContactSample sample (uint32_t milliseconds, adk::Level level,
                               adk::Status status = adk::StatusCode::Ok)
    {
        return {adk::TimePoint (milliseconds), level, status};
    }

    adk::ContactDynamicsConfig config (adk::Level active = adk::Level::High)
    {
        return {active, adk::Duration (5), adk::Duration (3), adk::Duration (20),
                adk::Duration (15)};
    }

    bool sameObservation (const adk::ContactObservation& left,
                          const adk::ContactObservation& right)
    {
        return left.observedAt == right.observedAt && left.rawLevel == right.rawLevel &&
               left.rawActive == right.rawActive &&
               left.qualifiedActive == right.qualifiedActive &&
               left.attackEvent == right.attackEvent &&
               left.releaseEvent == right.releaseEvent &&
               left.qualifiedPulseWidth == right.qualifiedPulseWidth &&
               left.refractoryRemaining == right.refractoryRemaining &&
               left.acceptedCount == right.acceptedCount &&
               left.suppressedCount == right.suppressedCount &&
               left.disposition == right.disposition && left.quality == right.quality &&
               left.status == right.status;
    }

    void testLifecycleConfigurationAndTraits ()
    {
        static_assert (!std::is_copy_constructible<adk::ContactDynamics>::value,
                       "not copy constructible");
        static_assert (!std::is_copy_assignable<adk::ContactDynamics>::value,
                       "not copy assignable");
        static_assert (!std::is_move_constructible<adk::ContactDynamics>::value,
                       "not move constructible");
        static_assert (!std::is_move_assignable<adk::ContactDynamics>::value,
                       "not move assignable");

        adk::ContactDynamics contact (config ());
        require                      (!contact.initialized (), "construction is inert");
        require                      (contact.update (sample (0, adk::Level::Low)).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejected");
        require (contact.initialize ().ok () && contact.initialize ().ok (),
                 "initialize succeeds and is idempotent");
        require (contact.snapshot ().quality == adk::ContactQuality::Unqualified,
                 "initialize manufactures no sample");

        const adk::Duration              valid (1);
        const adk::Duration              zero;
        const adk::Duration              half (0x80000000UL);
        const adk::ContactDynamicsConfig invalid[] = {
            {static_cast<adk::Level> (2), valid, valid, valid, valid},
            {adk::Level::High, zero, valid, valid, valid},
            {adk::Level::High, valid, zero, valid, valid},
            {adk::Level::High, valid, valid, zero, valid},
            {adk::Level::High, valid, valid, valid, zero},
            {adk::Level::High, half, valid, valid, valid},
            {adk::Level::High, valid, half, valid, valid},
            {adk::Level::High, valid, valid, half, valid},
            {adk::Level::High, valid, valid, valid, half}};

        for (const auto& config : invalid)
        {
            adk::ContactDynamics rejected (config);
            require                       (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid configuration rejected");
            require (!rejected.initialized (), "invalid policy remains inert");
        }
    }

    void testQualificationReleaseAndCanonicalFields ()
    {
        adk::ContactDynamics contact (config ());
        require                      (contact.initialize ().ok (), "fixture initializes");
        contact.update               (sample (10, adk::Level::Low));
        require                      (contact.snapshot ().quality == adk::ContactQuality::Valid &&
                     !contact.snapshot ().rawActive &&
                     contact.snapshot  ().qualifiedPulseWidth == adk::Duration () &&
                     contact.snapshot  ().refractoryRemaining == adk::Duration (),
                 "first idle sample has canonical fields");

        contact.update (sample (11, adk::Level::High));
        contact.update (sample (15, adk::Level::High));
        require        (!contact.snapshot ().attackEvent, "one tick before qualify");
        contact.update (sample (16, adk::Level::High));
        require        (
            contact.snapshot ().attackEvent && contact.snapshot ().qualifiedActive &&
                contact.snapshot ().disposition == adk::ContactDisposition::Accepted &&
                contact.snapshot ().acceptedCount == 1 &&
                contact.snapshot ().refractoryRemaining == adk::Duration (20),
            "attack qualifies at exact boundary");
        contact.update (sample (17, adk::Level::High));
        require        (!contact.snapshot ().attackEvent &&
                     contact.snapshot ().disposition == adk::ContactDisposition::None,
                 "one-update attack fields clear");

        contact.update (sample (20, adk::Level::Low));
        contact.update (sample (22, adk::Level::Low));
        require        (!contact.snapshot ().releaseEvent, "one tick before release");
        contact.update (sample (23, adk::Level::Low));
        require        (contact.snapshot ().releaseEvent &&
                     !contact.snapshot ().qualifiedActive &&
                     contact.snapshot  ().qualifiedPulseWidth == adk::Duration (7),
                 "release records accepted pulse width");
        contact.update (sample (24, adk::Level::Low));
        require        (!contact.snapshot ().releaseEvent &&
                     contact.snapshot ().qualifiedPulseWidth == adk::Duration (7),
                 "pulse width remains stable");

        contact.reset  ();
        contact.update (sample (30, adk::Level::High));
        contact.update (sample (36, adk::Level::High));
        require        (contact.snapshot ().attackEvent,
                 "one tick after qualification boundary qualifies");
        contact.update (sample (37, adk::Level::Low));
        contact.update (sample (41, adk::Level::Low));
        require        (contact.snapshot ().releaseEvent,
                 "one tick after release boundary releases");

        contact.reset  ();
        contact.update (sample (50, adk::Level::High));
        contact.update (sample (54, adk::Level::High));
        contact.update (sample (55, adk::Level::Low));
        contact.update (sample (60, adk::Level::Low));
        require        (contact.snapshot ().acceptedCount == 0,
                 "short pulse never qualifies");
    }

    void testBounceRefractoryAndExactEdge ()
    {
        adk::ContactDynamics contact (config (adk::Level::Low));
        require                      (contact.initialize ().ok (), "active-low fixture initializes");
        contact.update               (sample (0, adk::Level::High));
        contact.update               (sample (1, adk::Level::Low));
        contact.update               (sample (5, adk::Level::Low));
        contact.update               (sample (6, adk::Level::High));
        contact.update               (sample (7, adk::Level::Low));
        contact.update               (sample (11, adk::Level::Low));
        require                      (!contact.snapshot ().attackEvent, "bounce restarts qualification");
        contact.update               (sample (12, adk::Level::Low));
        require                      (contact.snapshot ().acceptedCount == 1, "active-low accepted");

        contact.update (sample (13, adk::Level::High));
        contact.update (sample (16, adk::Level::High));
        contact.update (sample (17, adk::Level::Low));
        contact.update (sample (22, adk::Level::Low));
        require        (contact.snapshot ().disposition ==
                         adk::ContactDisposition::SuppressedDuringRefractory &&
                     contact.snapshot ().suppressedCount == 1,
                 "attack before refractory edge suppressed");
        contact.update (sample (23, adk::Level::Low));
        require        (contact.snapshot ().suppressedCount == 1,
                 "held contact is not recounted");

        contact.reset  ();
        contact.update (sample (100, adk::Level::High));
        contact.update (sample (101, adk::Level::Low));
        contact.update (sample (106, adk::Level::Low));
        contact.update (sample (107, adk::Level::High));
        contact.update (sample (110, adk::Level::High));
        contact.update (sample (121, adk::Level::Low));
        contact.update (sample (126, adk::Level::Low));
        require        (contact.snapshot ().disposition == adk::ContactDisposition::Accepted &&
                     contact.snapshot ().acceptedCount == 2,
                 "attack at exact refractory edge accepted");

        contact.reset  ();
        contact.update (sample (200, adk::Level::Low));
        contact.update (sample (205, adk::Level::Low));
        contact.update (sample (206, adk::Level::High));
        contact.update (sample (209, adk::Level::High));
        contact.update (sample (221, adk::Level::Low));
        contact.update (sample (226, adk::Level::Low));
        require        (contact.snapshot ().disposition == adk::ContactDisposition::Accepted,
                 "attack one tick after refractory edge accepted");

        adk::ContactDynamics repeated (
            {adk::Level::High, adk::Duration (2), adk::Duration (1),
             adk::Duration (20), adk::Duration (30)});
        require         (repeated.initialize ().ok (), "repeated suppression initializes");
        repeated.update (sample (0, adk::Level::High));
        repeated.update (sample (2, adk::Level::High));
        repeated.update (sample (3, adk::Level::Low));
        repeated.update (sample (4, adk::Level::Low));
        repeated.update (sample (5, adk::Level::High));
        repeated.update (sample (7, adk::Level::High));
        repeated.update (sample (8, adk::Level::Low));
        repeated.update (sample (9, adk::Level::Low));
        repeated.update (sample (10, adk::Level::High));
        repeated.update (sample (12, adk::Level::High));
        require         (repeated.snapshot ().suppressedCount == 2,
                 "separate suppressed attacks are counted");
    }

    void testStuckFaultsAndTime ()
    {
        adk::ContactDynamics contact (config ());
        require                      (contact.initialize ().ok (), "stuck fixture initializes");
        contact.update               (sample (0, adk::Level::High));
        contact.update               (sample (5, adk::Level::High));
        contact.update               (sample (19, adk::Level::High));
        require                      (contact.snapshot ().quality == adk::ContactQuality::Valid,
                 "one tick before stuck valid");
        contact.update (sample (20, adk::Level::High));
        require        (contact.snapshot ().quality == adk::ContactQuality::StuckActive,
                 "stuck boundary reported");
        contact.update (sample (21, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::StuckActive,
                 "stuck remains until release qualifies");
        contact.update (sample (24, adk::Level::Low));
        require        (contact.snapshot ().releaseEvent &&
                     contact.snapshot ().quality == adk::ContactQuality::Valid,
                 "release precedes stuck and restores valid");

        contact.reset  ();
        contact.update (sample (30, adk::Level::High));
        contact.update (sample (35, adk::Level::High));
        contact.update (sample (47, adk::Level::Low));
        contact.update (sample (50, adk::Level::Low));
        require        (contact.snapshot ().releaseEvent &&
                     contact.snapshot ().quality == adk::ContactQuality::Valid,
                 "release wins exact collision with stuck boundary");

        contact.reset  ();
        contact.update (sample (10, adk::Level::High));
        require        (contact.update (sample (11, adk::Level::Low,
                                         adk::StatusCode::HardwareFailure))
                         .error () == adk::StatusCode::HardwareFailure,
                 "source failure propagates");
        contact.update (sample (20, adk::Level::High));
        require        (contact.snapshot ().quality == adk::ContactQuality::SourceFault,
                 "source fault requires reset");
        contact.update (sample (21, adk::Level::Low));
        contact.update (sample (21, adk::Level::High));
        require        (contact.snapshot ().quality == adk::ContactQuality::TimingFault,
                 "timing precedence applies after latched source fault");
        contact.update (sample (22, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::SourceFault &&
                     contact.snapshot ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                 "latched source status survives an intervening timing fault");
        contact.reset                        ();
        contact.update                       (sample (20, adk::Level::Low));
        const auto stable = contact.snapshot ();
        contact.update                       (sample (20, adk::Level::Low));
        require                              (sameObservation (stable, contact.snapshot ()),
                 "identical same-time sample idempotent");
        contact.update (sample (20, adk::Level::High));
        require        (contact.snapshot ().quality == adk::ContactQuality::TimingFault,
                 "changed same-time sample faults");
        contact.update (sample (21, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::Valid,
                 "timing fault does not mutate state");
        contact.update (sample (21, adk::Level::High,
                                adk::StatusCode::HardwareFailure));
        require        (contact.snapshot ().quality == adk::ContactQuality::TimingFault,
                 "timing fault precedes source fault at same time");

        contact.reset  ();
        contact.update (sample (0xfffffffcu, adk::Level::High));
        contact.update (sample (1, adk::Level::High));
        require        (contact.snapshot ().attackEvent, "rollover remains valid");
        contact.update (sample (0x80000001UL, adk::Level::High));
        require        (contact.snapshot ().quality == adk::ContactQuality::TimingFault,
                 "exact half range faults");

        contact.reset  ();
        contact.update (sample (0xfffffffeUL, adk::Level::Low));
        contact.update (sample (0xffffffffUL, adk::Level::Low));
        contact.update (sample (0, adk::Level::Low));
        contact.update (sample (1, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::Valid,
                 "frames immediately before at and after wrap are valid");

        contact.reset  ();
        contact.update (sample (0, adk::Level::Low));
        contact.update (sample (0x7fffffffUL, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::Valid,
                 "largest sub-half-range advance is valid");

        contact.reset  ();
        contact.update (sample (100, adk::Level::Low));
        contact.update (sample (99, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::TimingFault,
                 "backward apparent time faults");
    }

    void testMalformedLevelPrecedenceAndRecovery ()
    {
        adk::ContactDynamics contact (config ());
        require                      (contact.initialize ().ok (),
                 "malformed-level fixture initializes");
        contact.update                            (sample (0, adk::Level::High));
        contact.update                            (sample (5, adk::Level::High));
        const auto beforeFault = contact.snapshot ();

        require (contact.update (
                     sample (6, static_cast<adk::Level> (2),
                             adk::StatusCode::HardwareFailure))
                     .error () == adk::StatusCode::InvalidArgument,
                 "malformed level precedes source status");
        require (contact.snapshot ().quality == adk::ContactQuality::SourceFault &&
                     contact.snapshot ().observedAt == adk::TimePoint (6) &&
                     contact.snapshot ().qualifiedActive ==
                         beforeFault.qualifiedActive &&
                     contact.snapshot ().acceptedCount == beforeFault.acceptedCount &&
                     contact.snapshot ().suppressedCount ==
                         beforeFault.suppressedCount &&
                     contact.snapshot ().rawLevel == beforeFault.rawLevel &&
                     contact.snapshot ().rawActive == beforeFault.rawActive,
                 "malformed level latches without raw-policy mutation");

        contact.update (sample (7, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::SourceFault &&
                     contact.snapshot ().status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "malformed source fault requires reset");
        contact.reset  ();
        contact.update (sample (7, adk::Level::Low));
        require        (contact.snapshot ().quality == adk::ContactQuality::Valid &&
                     !contact.snapshot ().qualifiedActive &&
                     contact.snapshot  ().acceptedCount == 0,
                 "reset recovers from malformed source evidence");

        contact.update (sample (7, static_cast<adk::Level> (3)));
        require        (contact.snapshot ().quality == adk::ContactQuality::TimingFault,
                 "same-time mismatch precedes malformed-level validation");
    }

    void testSaturationAndReplay ()
    {
        adk::ContactDynamics saturated (config ());
        require                        (saturated.initialize ().ok (), "saturation fixture initializes");
        saturated.observation_.acceptedCount   = UINT32_MAX;
        saturated.observation_.suppressedCount = UINT32_MAX;
        saturated.update (sample (0, adk::Level::High));
        saturated.update (sample (5, adk::Level::High));
        saturated.update (sample (6, adk::Level::Low));
        saturated.update (sample (9, adk::Level::Low));
        saturated.update (sample (10, adk::Level::High));
        saturated.update (sample (15, adk::Level::High));
        require          (saturated.snapshot ().acceptedCount == UINT32_MAX &&
                     saturated.snapshot ().suppressedCount == UINT32_MAX,
                 "both counters saturate");

        adk::ContactDynamics left  (config ());
        adk::ContactDynamics right (config ());
        require                    (left.initialize ().ok () && right.initialize ().ok (),
                 "replay fixtures initialize");
        const adk::ContactSample trace[] = {
            sample (0, adk::Level::Low),  sample (1, adk::Level::High),
            sample (6, adk::Level::High), sample (7, adk::Level::Low),
            sample (10, adk::Level::Low), sample (30, adk::Level::High),
            sample (35, adk::Level::High)};
        for (const auto& frame : trace)
        {
            require (left.update (frame) == right.update (frame) &&
                         sameObservation (left.snapshot (), right.snapshot ()),
                     "identical copied traces replay exactly");
        }
    }
} // namespace
// clang-format on

int main ()
{
    testLifecycleConfigurationAndTraits        ();
    testQualificationReleaseAndCanonicalFields ();
    testBounceRefractoryAndExactEdge           ();
    testStuckFaultsAndTime                     ();
    testMalformedLevelPrecedenceAndRecovery    ();
    testSaturationAndReplay                    ();
    std::cout << "contact dynamics tests passed\n";
}
