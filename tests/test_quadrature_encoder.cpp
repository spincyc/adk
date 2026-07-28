#include <quadrature_encoder.h>

#include <Arduino.h>

#include <climits>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {
    namespace fake = adk::test::arduino;

    constexpr adk::PinId phaseAPin = 24;
    constexpr adk::PinId phaseBPin = 25;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void setPhase (uint8_t phase)
    {
        fake::setDigitalInput (phaseAPin, (phase & 0x02U) != 0 ? HIGH : LOW);
        fake::setDigitalInput (phaseBPin, (phase & 0x01U) != 0 ? HIGH : LOW);
    }

    void requireSnapshot (const adk::QuadratureEncoder& encoder, int32_t position,
                          int8_t delta, adk::Rotation rotation, uint8_t phase,
                          uint16_t invalidTransitions)
    {
        const adk::QuadratureEncoderSnapshot snapshot = encoder.snapshot ();

        require (snapshot.position == position, "position");
        require (snapshot.delta == delta, "delta");
        require (snapshot.rotation == rotation, "rotation");
        require (snapshot.phaseMask == phase, "phase mask");
        require (snapshot.invalidTransitions == invalidTransitions,
                 "invalid transition count");
    }

    void updatePhase (adk::QuadratureEncoder& encoder, uint8_t phase)
    {
        setPhase (phase);
        require  (encoder.update ().ok (), "phase update");
    }

    void testEveryStartingPhaseAndDirection ()
    {
        constexpr uint8_t clockwise[4]        = {0, 1, 3, 2};
        constexpr uint8_t counterClockwise[4] = {0, 2, 3, 1};

        for (uint8_t start = 0; start < 4; ++start)
        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});

            setPhase (clockwise[start]);
            require  (encoder.initialize ().ok (), "clockwise initialize");

            for (uint8_t edge = 1; edge <= 4; ++edge)
            {
                updatePhase     (encoder, clockwise[(start + edge) % 4U]);
                requireSnapshot (encoder, edge, 1, adk::Rotation::Clockwise,
                                 clockwise[(start + edge) % 4U], 0);
            }

            encoder.shutdown ();
            setPhase         (counterClockwise[start]);
            require          (encoder.initialize ().ok (), "counterclockwise initialize");

            for (uint8_t edge = 1; edge <= 4; ++edge)
            {
                updatePhase     (encoder, counterClockwise[(start + edge) % 4U]);
                requireSnapshot (encoder, static_cast<int32_t> (4 - edge), -1,
                                 adk::Rotation::CounterClockwise,
                                 counterClockwise[(start + edge) % 4U], 0);
            }
        }
    }

    void testPartialReverseBounceAndRepeatedStates ()
    {
        fake::reset ();
        adk::ResourceRegistry  resources;
        adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});

        setPhase (0);
        require  (encoder.initialize ().ok (), "sequence initialize");

        updatePhase     (encoder, 1);
        updatePhase     (encoder, 3);
        requireSnapshot (encoder, 2, 1, adk::Rotation::Clockwise, 3, 0);

        updatePhase     (encoder, 1);
        requireSnapshot (encoder, 1, -1, adk::Rotation::CounterClockwise, 1, 0);

        updatePhase     (encoder, 3);
        requireSnapshot (encoder, 2, 1, adk::Rotation::Clockwise, 3, 0);

        updatePhase     (encoder, 3);
        requireSnapshot (encoder, 2, 0, adk::Rotation::None, 3, 0);
    }

    void testReversedDirection ()
    {
        fake::reset ();
        adk::ResourceRegistry  resources;
        adk::QuadratureEncoder encoder (resources,
                                        {phaseAPin, phaseBPin, adk::Pull::Up, true});

        setPhase        (0);
        require         (encoder.initialize ().ok (), "reversed initialize");
        updatePhase     (encoder, 1);
        requireSnapshot (encoder, -1, -1, adk::Rotation::CounterClockwise, 1, 0);
        updatePhase     (encoder, 0);
        requireSnapshot (encoder, 0, 1, adk::Rotation::Clockwise, 0, 0);
    }

    void testEveryInvalidJumpRecovers ()
    {
        constexpr uint8_t opposite[4] = {3, 2, 1, 0};
        constexpr uint8_t next[4]     = {1, 3, 0, 2};

        for (uint8_t start = 0; start < 4; ++start)
        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});

            setPhase        (start);
            require         (encoder.initialize ().ok (), "invalid initialize");
            updatePhase     (encoder, opposite[start]);
            requireSnapshot (encoder, 0, 0, adk::Rotation::None, opposite[start], 1);

            updatePhase (encoder, next[opposite[start]]);
            require     (encoder.snapshot ().delta == 1, "recovery edge delta");
            require     (encoder.snapshot ().rotation == adk::Rotation::Clockwise,
                     "recovery edge direction");
        }
    }

    void testInvalidCounterSaturates ()
    {
        fake::reset ();
        adk::ResourceRegistry  resources;
        adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});

        setPhase (0);
        require  (encoder.initialize ().ok (), "counter initialize");

        for (uint32_t transition = 0; transition < 65537UL; ++transition)
        {
            updatePhase (encoder, (transition & 1U) == 0 ? 3 : 0);
        }

        require (encoder.snapshot ().invalidTransitions == UINT16_MAX,
                 "invalid counter saturation");
    }

    void testReadOrderAndReadCount ()
    {
        fake::reset ();
        adk::ResourceRegistry  resources;
        adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});

        setPhase (2);
        require  (encoder.initialize ().ok (), "read-order initialize");
        require  (fake::trace ().size () == 4, "initialize operation count");
        require  (fake::trace ()[0].kind == fake::OperationKind::PinMode,
                 "phase A configured first");
        require (fake::trace ()[0].pin == phaseAPin, "phase A mode pin");
        require (fake::trace ()[1].kind == fake::OperationKind::DigitalRead,
                 "phase A read first");
        require (fake::trace ()[1].pin == phaseAPin, "phase A read pin");
        require (fake::trace ()[2].kind == fake::OperationKind::PinMode,
                 "phase B configured second");
        require (fake::trace ()[2].pin == phaseBPin, "phase B mode pin");
        require (fake::trace ()[3].kind == fake::OperationKind::DigitalRead,
                 "phase B read second");
        require (fake::trace ()[3].pin == phaseBPin, "phase B read pin");

        fake::clearTrace ();
        updatePhase      (encoder, 3);
        require          (fake::trace ().size () == 2, "update read count");
        require          (fake::trace ()[0].kind == fake::OperationKind::DigitalRead,
                 "update reads A first");
        require (fake::trace ()[0].pin == phaseAPin, "update A pin");
        require (fake::trace ()[1].kind == fake::OperationKind::DigitalRead,
                 "update reads B second");
        require (fake::trace ()[1].pin == phaseBPin, "update B pin");
    }

    void testPositionSaturationAndReset ()
    {
        fake::reset ();
        adk::ResourceRegistry  resources;
        adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});

        setPhase (0);
        require  (encoder.initialize ().ok (), "saturation initialize");

        encoder.resetPosition (INT32_MAX);
        updatePhase           (encoder, 1);
        require               (encoder.snapshot ().position == INT32_MAX,
                 "maximum position saturates");
        require (encoder.snapshot ().delta == 1, "maximum preserves edge delta");
        require (encoder.snapshot ().positionSaturated, "maximum saturation flag");

        encoder.resetPosition ();
        require               (encoder.snapshot ().position == 0, "default reset");
        require               (!encoder.snapshot ().positionSaturated, "reset clears saturation");
        require               (encoder.snapshot ().phaseMask == 1, "reset preserves baseline");

        updatePhase           (encoder, 0);
        encoder.resetPosition (INT32_MIN);
        updatePhase           (encoder, 2);
        require               (encoder.snapshot ().position == INT32_MIN,
                 "minimum position saturates");
        require (encoder.snapshot ().delta == -1, "minimum preserves edge delta");
        require (encoder.snapshot ().positionSaturated, "minimum saturation flag");
    }

    void testConfigurationClaimsAndReuse ()
    {
        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder invalid (resources, {70, phaseBPin});
            require                        (invalid.initialize ().error () == adk::StatusCode::InvalidPin,
                     "invalid A pin");
            require (fake::trace ().empty (), "invalid A touches no hardware");
        }

        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder invalid (resources, {phaseAPin, 70});
            require                        (invalid.initialize ().error () == adk::StatusCode::InvalidPin,
                     "invalid B pin");
            require (fake::trace ().empty (), "invalid B touches no hardware");
        }

        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder duplicate (resources, {phaseAPin, phaseAPin});
            require                          (duplicate.initialize ().error () ==
                         adk::StatusCode::InvalidArgument,
                     "duplicate pins");
            require (fake::trace ().empty (), "duplicate touches no hardware");
        }

        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder invalidPull (
                resources, {phaseAPin, phaseBPin, static_cast<adk::Pull> (0xffU)});
            require (invalidPull.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid pull");
            require (fake::trace ().empty (), "invalid pull touches no hardware");
        }

        {
            fake::reset ();
            adk::ResourceRegistry resources;
            adk::DigitalInput     owner (resources, phaseAPin);
            require                     (owner.initialize ().ok (), "A owner initialize");
            fake::clearTrace            ();

            adk::QuadratureEncoder blocked (resources, {phaseAPin, phaseBPin});
            require                        (blocked.initialize ().error () == adk::StatusCode::ResourceBusy,
                     "busy A pin");
            require (fake::trace ().empty (), "busy A touches no encoder hardware");
        }

        {
            fake::reset ();
            adk::ResourceRegistry resources;
            adk::DigitalInput     owner (resources, phaseBPin);
            require                     (owner.initialize ().ok (), "B owner initialize");
            fake::clearTrace            ();

            adk::QuadratureEncoder blocked (resources, {phaseAPin, phaseBPin});
            require                        (blocked.initialize ().error () == adk::StatusCode::ResourceBusy,
                     "busy B pin");
            require (!resources.claimed ({adk::ResourceKind::Pin, phaseAPin}),
                     "busy B rolls A back");
            require (fake::mode (phaseAPin) == INPUT, "busy B leaves A safe");
            require (fake::trace ().size () == 3, "busy B rollback operations");
            require (fake::trace ()[0].kind == fake::OperationKind::PinMode,
                     "busy B configures A");
            require (fake::trace ()[1].kind == fake::OperationKind::DigitalRead,
                     "busy B establishes A");
            require (fake::trace ()[2].kind == fake::OperationKind::PinMode,
                     "busy B makes A safe");

            owner.shutdown ();
            setPhase       (0);
            require        (blocked.initialize ().ok (), "reuse after rollback");
        }
    }

    void testPullModesLifecycleAndDestruction ()
    {
        constexpr adk::Pull pulls[2] = {adk::Pull::None, adk::Pull::Up};
        constexpr uint8_t   modes[2] = {INPUT, INPUT_PULLUP};

        for (uint8_t index = 0; index < 2; ++index)
        {
            fake::reset ();
            adk::ResourceRegistry  resources;
            adk::QuadratureEncoder encoder (resources,
                                            {phaseAPin, phaseBPin, pulls[index]});

            setPhase (3);
            require  (!encoder.initialized (), "starts inactive");
            require  (encoder.update ().error () == adk::StatusCode::NotInitialized,
                     "inactive update");
            require         (fake::trace ().empty (), "inactive update touches no hardware");
            require         (encoder.initialize ().ok (), "pull initialize");
            require         (fake::mode (phaseAPin) == modes[index], "phase A pull mode");
            require         (fake::mode (phaseBPin) == modes[index], "phase B pull mode");
            requireSnapshot (encoder, 0, 0, adk::Rotation::None, 3, 0);

            fake::clearTrace ();
            require          (encoder.initialize ().ok (), "idempotent initialize");
            require          (fake::trace ().empty (),
                     "idempotent initialize touches no hardware");

            updatePhase      (encoder, 2);
            encoder.shutdown ();
            require          (!encoder.initialized (), "shutdown inactive");
            require          (fake::mode (phaseAPin) == INPUT, "shutdown A safe");
            require          (fake::mode (phaseBPin) == INPUT, "shutdown B safe");
            require          (!resources.claimed ({adk::ResourceKind::Pin, phaseAPin}),
                     "shutdown releases A");
            require (!resources.claimed ({adk::ResourceKind::Pin, phaseBPin}),
                     "shutdown releases B");
            require (encoder.snapshot ().position == 1, "shutdown retains position");
            require (encoder.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                     "shutdown status");

            fake::clearTrace ();
            encoder.shutdown ();
            require          (fake::trace ().empty (), "idempotent shutdown");
        }

        fake::reset ();
        adk::ResourceRegistry resources;

        {
            adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});
            setPhase                       (0);
            require                        (encoder.initialize ().ok (), "destruction initialize");
            fake::clearTrace               ();
        }

        require (fake::trace ().size () == 2, "destruction safe modes");
        require (fake::mode (phaseAPin) == INPUT, "destruction A safe");
        require (fake::mode (phaseBPin) == INPUT, "destruction B safe");

        adk::QuadratureEncoder replacement (resources, {phaseAPin, phaseBPin});
        require                            (replacement.initialize ().ok (), "destruction releases claims");
    }

    std::vector<adk::QuadratureEncoderSnapshot> replayTrace ()
    {
        constexpr uint8_t phases[] = {0, 1, 3, 1, 3, 2, 0, 3, 2};

        fake::reset ();
        adk::ResourceRegistry  resources;
        adk::QuadratureEncoder encoder (resources, {phaseAPin, phaseBPin});
        std::vector<adk::QuadratureEncoderSnapshot> snapshots;

        setPhase (phases[0]);
        require  (encoder.initialize ().ok (), "replay initialize");

        for (std::size_t index = 1; index < sizeof (phases); ++index)
        {
            updatePhase         (encoder, phases[index]);
            snapshots.push_back (encoder.snapshot ());
        }

        return snapshots;
    }

    void testDeterministicReplay ()
    {
        const auto first  = replayTrace ();
        const auto second = replayTrace ();

        require (first.size () == second.size (), "replay size");

        for (std::size_t index = 0; index < first.size (); ++index)
        {
            require (first[index].position == second[index].position,
                     "replay position");
            require (first[index].delta == second[index].delta, "replay delta");
            require (first[index].rotation == second[index].rotation,
                     "replay rotation");
            require (first[index].phaseMask == second[index].phaseMask, "replay phase");
            require (first[index].invalidTransitions ==
                         second[index].invalidTransitions,
                     "replay invalid count");
            require (first[index].positionSaturated == second[index].positionSaturated,
                     "replay saturation");
            require (first[index].status == second[index].status, "replay status");
        }
    }
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::QuadratureEncoder>::value, "");
    static_assert (!std::is_copy_assignable<adk::QuadratureEncoder>::value, "");
    static_assert (!std::is_move_constructible<adk::QuadratureEncoder>::value, "");
    static_assert (!std::is_move_assignable<adk::QuadratureEncoder>::value, "");

    testEveryStartingPhaseAndDirection        ();
    testPartialReverseBounceAndRepeatedStates ();
    testReversedDirection                     ();
    testEveryInvalidJumpRecovers              ();
    testInvalidCounterSaturates               ();
    testReadOrderAndReadCount                 ();
    testPositionSaturationAndReset            ();
    testConfigurationClaimsAndReuse           ();
    testPullModesLifecycleAndDestruction      ();
    testDeterministicReplay                   ();

    std::cout << "All ADK quadrature-encoder tests passed.\n";
}
