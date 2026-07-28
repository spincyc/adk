#include <magnetic_observation.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

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

    adk::LinearHallConfig hallConfig ()
    {
        return {54, 100, 900, 300, 400, 600, 700, adk::Duration (10), false};
    }

    void testHallLifecycleAndHysteresis ()
    {
        fake::reset ();

        fake::setAnalogInput (54, 500);

        adk::ResourceRegistry resources;
        adk::LinearHall       hall (resources, hallConfig ());

        require (!hall.initialized (), "hall starts inert");

        require (hall.initialize ().ok (), "hall initializes");

        require (hall.initialize ().ok (), "hall initialize is idempotent");

        hall.update (adk::TimePoint (0));

        require (hall.snapshot ().quality == adk::MagneticQuality::Valid,
                 "hall publishes valid sample");

        require (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "hall starts neutral");

        fake::setAnalogInput (54, 300);

        hall.update (adk::TimePoint (1));

        hall.update (adk::TimePoint (11));

        require (hall.snapshot ().polarity == adk::MagneticPolarity::Negative,
                 "negative threshold qualifies after dwell");

        require (hall.snapshot ().activationEvent, "activation event emitted");

        fake::setAnalogInput (54, 350);

        hall.update (adk::TimePoint (12));

        require (hall.snapshot ().polarity == adk::MagneticPolarity::Negative,
                 "negative hysteresis holds");

        fake::setAnalogInput (54, 500);

        hall.update (adk::TimePoint (13));

        hall.update (adk::TimePoint (23));

        require (hall.snapshot ().polarity == adk::MagneticPolarity::Neutral,
                 "neutral qualifies");

        require (hall.snapshot ().deactivationEvent, "deactivation event emitted");

        fake::setAnalogInput (54, 901);

        hall.update (adk::TimePoint (24));

        require (hall.snapshot ().quality == adk::MagneticQuality::AboveQualifiedRange,
                 "out-of-range evidence retained");

        require (hall.snapshot ().raw == 901, "raw evidence retained");

        const auto before = hall.snapshot ();

        hall.update (adk::TimePoint (23));

        require (hall.snapshot ().status.error () == adk::StatusCode::InvalidArgument,
                 "backward time rejected");

        require (hall.snapshot ().raw == before.raw, "backward time does not resample");

        hall.shutdown ();

        hall.shutdown ();

        require (!hall.initialized (), "hall shuts down idempotently");
    }

    void testContactDwellAndRollover ()
    {
        fake::reset ();

        fake::setDigitalInput (22, HIGH);

        adk::ResourceRegistry            resources;
        const adk::MagneticContactConfig config = {22, adk::Pull::Up, adk::Level::Low,
                                                   adk::Duration (5)};
        adk::MagneticContact             contact (resources, config);

        require (contact.initialize ().ok (), "contact initializes");

        contact.update (adk::TimePoint (0xfffffffcu));

        require (!contact.snapshot ().active, "open contact is inactive");

        fake::setDigitalInput (22, LOW);

        contact.update (adk::TimePoint (0xfffffffeu));

        contact.update (adk::TimePoint (3));

        require (contact.snapshot ().active, "rollover dwell qualifies");

        require (contact.snapshot ().activationEvent,
                 "contact activation event emitted");

        require (contact.snapshot ().raw == 0, "low canonical raw value");

        require (contact.snapshot ().polarity == adk::MagneticPolarity::Unspecified,
                 "contact does not invent polarity");
    }

    void testValidationAndOwnership ()
    {
        static_assert (!std::is_copy_constructible<adk::LinearHall>::value,
                       "hall owns its input");
        static_assert (!std::is_move_constructible<adk::MagneticContact>::value,
                       "contact owns its input");

        fake::reset ();

        adk::ResourceRegistry resources;
        auto                  invalid = hallConfig ();
        invalid.negativeRelease       = invalid.negativeActivate;
        adk::LinearHall hall (resources, invalid);

        require (hall.initialize ().error () == adk::StatusCode::InvalidConfiguration,
                 "invalid threshold ordering rejected");

        adk::LinearHall first (resources, hallConfig ());

        adk::LinearHall second (resources, hallConfig ());

        require (first.initialize ().ok (), "first hall owns pin");

        require (second.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "busy analog pin rejected");

        first.shutdown ();

        require (second.initialize ().ok (), "released pin reusable");
    }
} // namespace

int main ()
{
    testHallLifecycleAndHysteresis ();

    testContactDwellAndRollover ();

    testValidationAndOwnership ();
    std::cout << "magnetic observation tests passed\n";
    return EXIT_SUCCESS;
}
