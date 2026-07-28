#include <magnetic_observation.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {
    namespace fake = adk::test::arduino;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::LinearHallConfig hallConfig (adk::Duration dwell   = adk::Duration (10),
                                      bool          reverse = false)
    {
        return {54, 100, 900, 300, 400, 600, 700, dwell, reverse};
    }

    adk::MagneticContactConfig contactConfig (adk::Duration dwell  = adk::Duration (5),
                                              adk::Pull     pull   = adk::Pull::Up,
                                              adk::Level    closed = adk::Level::Low)
    {
        return {22, pull, closed, dwell};
    }

    void requireObservationEqual (const adk::MagneticObservation& left,
                                  const adk::MagneticObservation& right,
                                  const char*                     message)
    {
        require (left.source == right.source && left.raw == right.raw &&
                     left.rawLevel == right.rawLevel &&
                     left.observedAt.milliseconds () ==
                         right.observedAt.milliseconds () &&
                     left.polarity == right.polarity &&
                     left.activationEvent == right.activationEvent &&
                     left.deactivationEvent == right.deactivationEvent &&
                     left.active == right.active && left.stableFor == right.stableFor &&
                     left.quality == right.quality &&
                     left.status.error () == right.status.error (),
                 message);
    }

    void requireOneRead (fake::OperationKind kind, uint8_t pin, const char* message)
    {
        require (fake::trace ().size () == 1, message);
        require (fake::trace ()[0].kind == kind, message);
        require (fake::trace ()[0].pin == pin, message);
    }

    void hallUpdate (adk::LinearHall& hall, uint16_t raw, uint32_t now)
    {
        fake::setAnalogInput (54, raw);
        fake::clearTrace     ();
        hall.update          (adk::TimePoint (now));
        requireOneRead       (fake::OperationKind::AnalogRead, 54,
                        "hall update samples exactly once");
    }

    void contactUpdate (adk::MagneticContact& contact, uint8_t level, uint32_t now)
    {
        fake::setDigitalInput (22, level);
        fake::clearTrace      ();
        contact.update        (adk::TimePoint (now));
        requireOneRead        (fake::OperationKind::DigitalRead, 22,
                        "contact update samples exactly once");
    }

    void testHallThresholdsAndHysteresis ()
    {
        fake::reset          ();
        fake::setAnalogInput (54, 500);
        adk::ResourceRegistry resources;
        adk::LinearHall       hall (resources, hallConfig ());

        require (!hall.initialized (), "hall starts inert");
        require (hall.snapshot ().quality == adk::MagneticQuality::Unqualified,
                 "hall starts unqualified");
        require (hall.initialize ().ok (), "hall initializes");
        require (hall.initialize ().ok (), "hall initialize is idempotent");
        require (hall.snapshot ().status.error () == adk::StatusCode::NotInitialized,
                 "hall waits for first explicit update");

        hallUpdate (hall, 301, 0);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "negative threshold plus one stays neutral");
        hallUpdate (hall, 300, 1);
        hallUpdate (hall, 300, 11);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Negative,
                 "exact negative activation qualifies");
        require (hall.snapshot ().activationEvent, "negative activation event");

        hallUpdate (hall, 399, 12);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Negative,
                 "negative release minus one holds");
        hallUpdate (hall, 400, 13);
        hallUpdate (hall, 400, 23);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "exact negative release qualifies neutral");
        require (hall.snapshot ().deactivationEvent, "negative deactivation event");

        hallUpdate (hall, 699, 24);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "positive threshold minus one stays neutral");
        hallUpdate (hall, 700, 25);
        hallUpdate (hall, 700, 35);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Positive,
                 "exact positive activation qualifies");

        hallUpdate (hall, 601, 36);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Positive,
                 "positive release plus one holds");
        hallUpdate (hall, 600, 37);
        hallUpdate (hall, 600, 47);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "exact positive release qualifies neutral");
    }

    void testHallReverseAndDirectOpposite ()
    {
        fake::reset          ();
        fake::setAnalogInput (54, 500);
        adk::ResourceRegistry resources;
        adk::LinearHall       hall (resources, hallConfig (adk::Duration (10), true));
        require                    (hall.initialize ().ok (), "reverse hall initializes");

        hallUpdate (hall, 300, 0);
        hallUpdate (hall, 300, 10);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Positive,
                 "reversal swaps negative report");

        hallUpdate (hall, 700, 11);
        hallUpdate (hall, 700, 21);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "direct opposite first qualifies neutral");
        require (hall.snapshot ().deactivationEvent,
                 "direct opposite emits deactivation first");
        require (!hall.snapshot ().activationEvent,
                 "direct opposite does not activate concurrently");

        hallUpdate (hall, 700, 22);
        hallUpdate (hall, 700, 32);
        require    (hall.snapshot ().polarity == adk::MagneticPolarity::Negative,
                 "direct opposite qualifies reversed new polarity separately");
        require (hall.snapshot ().activationEvent,
                 "direct opposite later emits activation");
    }

    void testHallZeroDwellAndRangeRecovery ()
    {
        fake::reset          ();
        fake::setAnalogInput (54, 500);
        adk::ResourceRegistry resources;
        adk::LinearHall       hall (resources, hallConfig (adk::Duration (0)));
        require                    (hall.initialize ().ok (), "zero-dwell hall initializes");

        hallUpdate (hall, 300, 0);
        require    (hall.snapshot ().active && hall.snapshot ().activationEvent,
                 "zero dwell qualifies on first update");

        hallUpdate (hall, 500, 1);
        require    (!hall.snapshot ().active && hall.snapshot ().deactivationEvent,
                 "zero dwell releases on first update");

        adk::LinearHall ranged (resources, hallConfig ());
        hall.shutdown          ();
        require                (ranged.initialize ().ok (), "range recovery hall initializes");
        hallUpdate             (ranged, 300, 0);
        hallUpdate             (ranged, 99, 5);
        require                (ranged.snapshot ().quality ==
                     adk::MagneticQuality::BelowQualifiedRange,
                 "low out-of-range retained");
        hallUpdate (ranged, 300, 10);
        require    (!ranged.snapshot ().active, "out-of-range clears pending candidate");
        hallUpdate (ranged, 300, 20);
        require    (ranged.snapshot ().active, "candidate restarts after valid recovery");
        hallUpdate (ranged, 901, 21);
        require    (ranged.snapshot ().quality ==
                     adk::MagneticQuality::AboveQualifiedRange,
                 "high out-of-range retained");
        require (ranged.snapshot ().raw == 901 &&
                     ranged.snapshot ().rawLevel == adk::Level::Low,
                 "hall retains canonical raw evidence");
    }

    void testHallTimeAndReplay ()
    {
        fake::reset          ();
        fake::setAnalogInput (54, 300);
        adk::ResourceRegistry resources;
        adk::LinearHall       hall (resources, hallConfig ());
        require                    (hall.initialize ().ok (), "timed hall initializes");
        hallUpdate                 (hall, 300, 0xfffffff5u);
        hallUpdate                 (hall, 300, 0xffffffffu);
        require                    (hall.snapshot ().active, "hall dwell crosses rollover");

        const auto before = hall.snapshot ();
        fake::clearTrace                  ();
        hall.update                       (adk::TimePoint (0x7fffffffu));
        require                           (fake::trace ().empty (), "half-range hall time does not sample");
        require                           (hall.snapshot ().status.error () == adk::StatusCode::InvalidArgument,
                 "half-range hall time rejected");
        require (hall.snapshot ().activationEvent == before.activationEvent &&
                     hall.snapshot ().raw == before.raw,
                 "invalid hall time preserves event and evidence");

        fake::clearTrace ();
        hall.update      (adk::TimePoint (0xfffffffeu));
        require          (fake::trace ().empty (), "backward hall time does not sample");

        auto run = [] ()
        {
            fake::reset          ();
            fake::setAnalogInput (54, 500);
            adk::ResourceRegistry replayResources;
            adk::LinearHall       replay (replayResources, hallConfig ());
            require                      (replay.initialize ().ok (), "replay hall initializes");
            hallUpdate                   (replay, 300, 1);
            hallUpdate                   (replay, 300, 11);
            return replay.snapshot       ();
        };
        const auto first  = run ();
        const auto second = run ();
        requireObservationEqual (first, second, "hall replay is field-identical");
    }

    void testContactDwellBounceVariantsAndTime ()
    {
        fake::reset           ();
        fake::setDigitalInput (22, HIGH);
        adk::ResourceRegistry resources;
        adk::MagneticContact  contact (resources, contactConfig ());
        require                       (contact.initialize ().ok (), "contact initializes");
        require                       (contact.snapshot ().status.error () == adk::StatusCode::NotInitialized,
                 "contact waits for explicit update");

        contactUpdate (contact, HIGH, 0xfffffffcu);
        contactUpdate (contact, LOW, 0xfffffffeu);
        contactUpdate (contact, HIGH, 0);
        contactUpdate (contact, LOW, 1);
        require       (!contact.snapshot ().active, "contact bounce resets dwell");
        contactUpdate (contact, LOW, 6);
        require       (contact.snapshot ().active && contact.snapshot ().activationEvent,
                 "contact qualifies across rollover after stable dwell");
        require (contact.snapshot ().raw == 0 &&
                     contact.snapshot ().rawLevel == adk::Level::Low &&
                     contact.snapshot ().polarity == adk::MagneticPolarity::Unspecified,
                 "contact publishes canonical low evidence");

        const auto before = contact.snapshot ();
        fake::clearTrace                     ();
        contact.update                       (adk::TimePoint (0x80000006u));
        require                              (fake::trace ().empty (), "half-range contact time does not sample");
        require                              (contact.snapshot ().activationEvent == before.activationEvent,
                 "invalid contact time preserves event");

        fake::clearTrace ();
        contact.update   (adk::TimePoint (5));
        require          (fake::trace ().empty (), "backward contact time does not sample");

        contact.shutdown                ();
        adk::MagneticContact highClosed (
            resources,
            contactConfig (adk::Duration (0), adk::Pull::None, adk::Level::High));
        require       (highClosed.initialize ().ok (), "high-closed contact initializes");
        contactUpdate (highClosed, HIGH, 0);
        require       (highClosed.snapshot ().active && highClosed.snapshot ().raw == 1,
                 "high-closed no-pull contact activates");
        contactUpdate (highClosed, LOW, 1);
        require       (!highClosed.snapshot ().active &&
                     highClosed.snapshot ().deactivationEvent,
                 "zero-dwell high-closed contact deactivates");
    }

    void testValidationOwnershipAndLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::LinearHall>::value, "");
        static_assert (!std::is_copy_assignable<adk::LinearHall>::value, "");
        static_assert (!std::is_move_constructible<adk::LinearHall>::value, "");
        static_assert (!std::is_move_assignable<adk::LinearHall>::value, "");
        static_assert (!std::is_copy_constructible<adk::MagneticContact>::value, "");
        static_assert (!std::is_copy_assignable<adk::MagneticContact>::value, "");
        static_assert (!std::is_move_constructible<adk::MagneticContact>::value, "");
        static_assert (!std::is_move_assignable<adk::MagneticContact>::value, "");

        fake::reset ();
        adk::ResourceRegistry resources;
        const auto            expectInvalidHall = [&] (adk::LinearHallConfig config)
        {
            fake::clearTrace     ();
            adk::LinearHall hall (resources, config);
            require              (hall.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid hall configuration rejected");
            require (fake::trace ().empty (), "invalid hall touches no hardware");
        };

        auto invalid             = hallConfig ();
        invalid.qualifiedMinimum = 301;
        expectInvalidHall                    (invalid);
        invalid                 = hallConfig ();
        invalid.negativeRelease = 300;
        expectInvalidHall                    (invalid);
        invalid                 = hallConfig ();
        invalid.positiveRelease = 400;
        expectInvalidHall                     (invalid);
        invalid                  = hallConfig ();
        invalid.positiveActivate = 600;
        expectInvalidHall                     (invalid);
        invalid                  = hallConfig ();
        invalid.qualifiedMaximum = 699;
        expectInvalidHall                     (invalid);
        invalid                  = hallConfig ();
        invalid.qualifiedMaximum = 1024;
        expectInvalidHall             (invalid);
        invalid       = hallConfig    ();
        invalid.dwell = adk::Duration (0x80000000u);
        expectInvalidHall             (invalid);

        const adk::MagneticContactConfig invalidContacts[] = {
            {22, static_cast<adk::Pull> (2), adk::Level::Low, adk::Duration (1)},
            {22, adk::Pull::Up, static_cast<adk::Level> (2), adk::Duration  (1)},
            {22, adk::Pull::Up, adk::Level::Low, adk::Duration              (0x80000000u)}};
        for (const auto& config : invalidContacts)
        {
            fake::clearTrace             ();
            adk::MagneticContact contact (resources, config);
            require                      (contact.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid contact configuration rejected");
            require (fake::trace ().empty (), "invalid contact touches no hardware");
        }

        adk::LinearHall unsupported (
            resources, {53, 100, 900, 300, 400, 600, 700, adk::Duration (1), false});
        require (unsupported.initialize ().error () == adk::StatusCode::Unsupported,
                 "unsupported hall pin rejected");
        adk::LinearHall invalidPin (
            resources, {70, 100, 900, 300, 400, 600, 700, adk::Duration (1), false});
        require (invalidPin.initialize ().error () == adk::StatusCode::InvalidPin,
                 "invalid hall pin rejected");

        adk::LinearHall first  (resources, hallConfig ());
        adk::LinearHall second (resources, hallConfig ());
        require                (first.initialize ().ok (), "first hall owns pin");
        fake::clearTrace       ();
        require                (second.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "busy hall pin rejected");
        require         (fake::trace ().empty (), "busy hall touches no hardware");
        first.shutdown  ();
        require         (second.initialize ().ok (), "hall pin reusable after shutdown");
        second.shutdown ();
        second.shutdown ();

        adk::MagneticContact firstContact  (resources, contactConfig ());
        adk::MagneticContact secondContact (resources, contactConfig ());
        require                            (firstContact.initialize ().ok (), "first contact owns pin");
        fake::clearTrace                   ();
        require                            (secondContact.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "busy contact pin rejected");
        require               (fake::trace ().empty (), "busy contact touches no hardware");
        firstContact.shutdown ();
        require               (secondContact.initialize ().ok (),
                 "contact pin reusable after shutdown");
        secondContact.shutdown ();
        require                (secondContact.snapshot ().status.error () ==
                     adk::StatusCode::NotInitialized,
                 "contact shutdown publishes not initialized");
    }
} // namespace

int main ()
{
    testHallThresholdsAndHysteresis       ();
    testHallReverseAndDirectOpposite      ();
    testHallZeroDwellAndRangeRecovery     ();
    testHallTimeAndReplay                 ();
    testContactDwellBounceVariantsAndTime ();
    testValidationOwnershipAndLifecycle   ();
    std::cout << "magnetic observation tests passed\n";
    return EXIT_SUCCESS;
}
