#include <inert_load_interlock.h>
#include <power_domain.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct TestPower final : adk::PowerDomain
    {
        bool commandAdmitted () const noexcept override
        {
            return admitted;
        }

        bool admitted = true;
    };

    void requireSnapshot (const adk::InertLoadInterlock& interlock,
                          adk::SimulatedLoad requested, adk::SimulatedLoad active,
                          adk::Status status, const char* message)
    {
        const adk::InertLoadSnapshot snapshot = interlock.snapshot ();

        require (snapshot.requested == requested, message);
        require (snapshot.active == active, message);
        require (snapshot.status == status, message);
    }

    void testLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::InertLoadInterlock>::value,
                       "inert load interlock must not copy");
        static_assert (!std::is_move_constructible<adk::InertLoadInterlock>::value,
                       "inert load interlock must not move");

        TestPower               power;
        adk::InertLoadInterlock interlock (power);

        require         (!interlock.initialized (), "construction is inert");
        requireSnapshot (interlock, adk::SimulatedLoad::None, adk::SimulatedLoad::None,
                         adk::StatusCode::Ok, "construction selects no load");
        require (interlock.select (adk::SimulatedLoad::Pump).error () ==
                     adk::StatusCode::NotInitialized,
                 "selection before initialization is rejected");
        require (interlock.initialize ().ok (), "initialization succeeds");
        require (interlock.initialize ().ok (), "initialization is idempotent");

        interlock.shutdown ();
        interlock.shutdown ();
        require            (!interlock.initialized (), "shutdown clears initialization");
        requireSnapshot    (interlock, adk::SimulatedLoad::None, adk::SimulatedLoad::None,
                         adk::StatusCode::Ok, "shutdown selects no load");
    }

    void testEverySelectionTransition ()
    {
        const adk::SimulatedLoad loads[] = {
            adk::SimulatedLoad::None, adk::SimulatedLoad::Fan, adk::SimulatedLoad::Pump,
            adk::SimulatedLoad::Heater};
        TestPower               power;
        adk::InertLoadInterlock interlock (power);

        require (interlock.initialize ().ok (), "interlock initializes");

        for (const adk::SimulatedLoad from : loads)
        {
            require (interlock.select (from).ok (), "starting selection succeeds");

            for (const adk::SimulatedLoad to : loads)
            {
                require         (interlock.select (to).ok (), "transition succeeds");
                requireSnapshot (interlock, to, to, adk::StatusCode::Ok,
                                 "only target is active");
            }
        }
    }

    void testInvalidSelectionDoesNotChangeState ()
    {
        TestPower               power;
        adk::InertLoadInterlock interlock (power);

        require (interlock.initialize ().ok (), "interlock initializes");
        require (interlock.select (adk::SimulatedLoad::Fan).ok (), "fan selected");
        require (interlock.select (static_cast<adk::SimulatedLoad> (255)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "unknown selection rejected");
        requireSnapshot (interlock, adk::SimulatedLoad::Fan, adk::SimulatedLoad::Fan,
                         adk::StatusCode::Ok, "invalid selection preserves state");
    }

    void testRevokedAdmissionFailsClosedAndStaysFaulted ()
    {
        TestPower               power;
        adk::InertLoadInterlock interlock (power);

        require (interlock.initialize ().ok (), "interlock initializes");
        require (interlock.select (adk::SimulatedLoad::Fan).ok (), "fan selected");

        power.admitted = false;

        const adk::Status status = interlock.select (adk::SimulatedLoad::Pump);

        require (status.error () == adk::StatusCode::HardwareFailure,
                 "revoked admission reports hardware failure");
        require         (status.transient (), "admission failure is classified transient");
        requireSnapshot (interlock, adk::SimulatedLoad::Pump, adk::SimulatedLoad::None,
                         adk::StatusCode::HardwareFailure,
                         "revoked admission removes every active intent");

        power.admitted = true;

        require (interlock.select (adk::SimulatedLoad::Heater).error () ==
                     adk::StatusCode::HardwareFailure,
                 "fault remains sticky after admission returns");
        requireSnapshot (interlock, adk::SimulatedLoad::Heater,
                         adk::SimulatedLoad::None, adk::StatusCode::HardwareFailure,
                         "sticky fault cannot reactivate a load");
        require (interlock.select (adk::SimulatedLoad::None).error () ==
                     adk::StatusCode::HardwareFailure,
                 "safe selection preserves fault evidence");
        requireSnapshot (interlock, adk::SimulatedLoad::None, adk::SimulatedLoad::None,
                         adk::StatusCode::HardwareFailure,
                         "safe selection remains inactive");
    }

    void testExplicitRecovery ()
    {
        TestPower               power;
        adk::InertLoadInterlock interlock (power);

        require (interlock.initialize ().ok (), "interlock initializes");
        power.admitted = false;
        require (!interlock.select (adk::SimulatedLoad::Pump).ok (),
                 "revoked admission faults");

        interlock.shutdown ();
        power.admitted = true;

        require (interlock.initialize ().ok (), "lifecycle restart clears fault");
        require (interlock.select (adk::SimulatedLoad::Pump).ok (),
                 "selection succeeds after explicit recovery");
        requireSnapshot (interlock, adk::SimulatedLoad::Pump, adk::SimulatedLoad::Pump,
                         adk::StatusCode::Ok, "recovery permits one bounded intent");
    }

    void testRevocationIsObservedWithoutAnotherSelection ()
    {
        TestPower               power;
        adk::InertLoadInterlock interlock (power);

        require (interlock.initialize ().ok (), "interlock initializes");
        require (interlock.select (adk::SimulatedLoad::Heater).ok (),
                 "heater selected");

        power.admitted = false;

        require (interlock.update ().error () == adk::StatusCode::HardwareFailure,
                 "update observes asynchronous admission revocation");
        requireSnapshot (interlock, adk::SimulatedLoad::Heater,
                         adk::SimulatedLoad::None, adk::StatusCode::HardwareFailure,
                         "revocation fails active intent closed");
    }

    void testDestructionIsInert ()
    {
        TestPower power;

        {
            adk::InertLoadInterlock interlock (power);

            require (interlock.initialize ().ok (), "interlock initializes");
            require (interlock.select (adk::SimulatedLoad::Heater).ok (),
                     "heater intent selected");
        }

        require (power.admitted, "destruction does not mutate admission");
    }
} // namespace

int main ()
{
    testLifecycle                                   ();
    testEverySelectionTransition                    ();
    testInvalidSelectionDoesNotChangeState          ();
    testRevokedAdmissionFailsClosedAndStaysFaulted  ();
    testExplicitRecovery                            ();
    testRevocationIsObservedWithoutAnotherSelection ();
    testDestructionIsInert                          ();
}
