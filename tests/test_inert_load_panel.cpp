#include <inert_load_interlock.h>
#include <inert_load_panel.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct RecordingOutput final : adk::PumpOutput
    {
        adk::Status initialize () noexcept override
        {
            calls.push_back (2);

            if (!initializeStatus.ok ())
            {
                return initializeStatus;
            }

            initialized_ = true;
            state_       = adk::PumpState::Off;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            calls.push_back (3);
            initialized_ = false;
            state_       = adk::PumpState::Off;
        }

        adk::Status setState (adk::PumpState state) noexcept override
        {
            calls.push_back (state == adk::PumpState::On ? 1 : 0);

            if (!initialized_)
            {
                return adk::StatusCode::NotInitialized;
            }

            if (failNextSet)
            {
                failNextSet = false;
                return adk::StatusCode::HardwareFailure;
            }

            state_ = state;
            return adk::StatusCode::Ok;
        }

        adk::PumpState state () const noexcept override
        {
            return state_;
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

        adk::Status      initializeStatus = adk::StatusCode::Ok;
        std::vector<int> calls;
        adk::PumpState   state_       = adk::PumpState::Off;
        bool             initialized_ = false;
        bool             failNextSet  = false;
    };

    void requireAllOff (const RecordingOutput& fan, const RecordingOutput& pump,
                        const RecordingOutput& heater, const char* message)
    {
        require (fan.state () == adk::PumpState::Off, message);
        require (pump.state () == adk::PumpState::Off, message);
        require (heater.state () == adk::PumpState::Off, message);
    }

    void testTransactionalInitialization ()
    {
        for (uint8_t failure = 0; failure < 3; ++failure)
        {
            RecordingOutput  fan;
            RecordingOutput  pump;
            RecordingOutput  heater;
            RecordingOutput* outputs[] = {&fan, &pump, &heater};

            outputs[failure]->initializeStatus = adk::StatusCode::ResourceBusy;

            adk::InertLoadPanel panel (fan, pump, heater);

            require (panel.initialize ().error () == adk::StatusCode::ResourceBusy,
                     "initialization failure is propagated");
            require (!panel.initialized (), "partial initialization is not exposed");

            for (RecordingOutput* output : outputs)
            {
                require (!output->initialized (),
                         "every earlier output is rolled back");
            }
        }
    }

    void testPreinitializedBorrowedOutputIsUntouched ()
    {
        RecordingOutput fan;
        RecordingOutput pump;
        RecordingOutput heater;

        require (pump.initialize ().ok (), "borrowed pump is preinitialized");

        const std::size_t pumpCalls = pump.calls.size ();
        adk::InertLoadPanel panel                     (fan, pump, heater);

        require (panel.initialize ().error () == adk::StatusCode::InvalidArgument,
                 "panel rejects a preinitialized borrowed output");
        require (!fan.initialized () && pump.initialized () && !heater.initialized (),
                 "precondition failure preserves every borrowed lifecycle");
        require (pump.calls.size () == pumpCalls,
                 "precondition failure emits no borrowed-output operation");
    }

    void testEveryTransitionIsExclusive ()
    {
        const adk::SimulatedLoad loads[] = {
            adk::SimulatedLoad::None, adk::SimulatedLoad::Fan, adk::SimulatedLoad::Pump,
            adk::SimulatedLoad::Heater};
        RecordingOutput     fan;
        RecordingOutput     pump;
        RecordingOutput     heater;
        adk::InertLoadPanel panel (fan, pump, heater);

        require (panel.initialize ().ok (), "panel initializes");

        for (const adk::SimulatedLoad from : loads)
        {
            require (panel.select (from).ok (), "source selection succeeds");

            for (const adk::SimulatedLoad to : loads)
            {
                require (panel.select (to).ok (), "target selection succeeds");
                const unsigned int activeCount =
                    static_cast<uint8_t> (fan.state    () == adk::PumpState::On) +
                    static_cast<uint8_t> (pump.state   () == adk::PumpState::On) +
                    static_cast<uint8_t> (heater.state () == adk::PumpState::On);

                require (activeCount == (to == adk::SimulatedLoad::None ? 0 : 1),
                         "no transition exposes two active indicators");
                require (panel.snapshot ().active == to,
                         "snapshot matches exclusive output");
            }
        }
    }

    void testUnchangedSelectionDoesNotWrite ()
    {
        RecordingOutput     fan;
        RecordingOutput     pump;
        RecordingOutput     heater;
        adk::InertLoadPanel panel (fan, pump, heater);

        require (panel.initialize ().ok (), "panel initializes");
        require (panel.select (adk::SimulatedLoad::Pump).ok (), "pump selected");

        const std::size_t fanCalls    = fan.calls.size    ();
        const std::size_t pumpCalls   = pump.calls.size   ();
        const std::size_t heaterCalls = heater.calls.size ();

        require (panel.select (adk::SimulatedLoad::Pump).ok (),
                 "unchanged selection succeeds");
        require (fan.calls.size () == fanCalls && pump.calls.size () == pumpCalls &&
                     heater.calls.size () == heaterCalls,
                 "unchanged selection emits no command");
    }

    void testTransitionFailuresMakeAllOff ()
    {
        RecordingOutput     fan;
        RecordingOutput     pump;
        RecordingOutput     heater;
        adk::InertLoadPanel panel (fan, pump, heater);

        require (panel.initialize ().ok (), "panel initializes");
        require (panel.select (adk::SimulatedLoad::Fan).ok (), "fan selected");

        fan.failNextSet = true;
        require (panel.select (adk::SimulatedLoad::Pump).error () ==
                     adk::StatusCode::HardwareFailure,
                 "old-off failure is normalized");
        requireAllOff (fan, pump, heater, "old-off failure rolls every output off");
        require       (panel.snapshot ().active == adk::SimulatedLoad::None,
                 "old-off failure snapshot is safe");

        require (panel.select (adk::SimulatedLoad::Fan).ok (), "panel can retry");
        pump.failNextSet = true;
        require (panel.select (adk::SimulatedLoad::Pump).error () ==
                     adk::StatusCode::HardwareFailure,
                 "new-on failure is normalized");
        requireAllOff (fan, pump, heater, "new-on failure rolls every output off");
    }

    void testLifecycleAndInvalidSelection ()
    {
        static_assert (!std::is_copy_constructible<adk::InertLoadPanel>::value,
                       "panel must not copy");
        static_assert (!std::is_move_constructible<adk::InertLoadPanel>::value,
                       "panel must not move");

        RecordingOutput fan;
        RecordingOutput pump;
        RecordingOutput heater;

        {
            adk::InertLoadPanel panel (fan, pump, heater);

            require (panel.select (adk::SimulatedLoad::Fan).error () ==
                         adk::StatusCode::NotInitialized,
                     "selection before initialization is rejected");
            require (panel.initialize ().ok (), "panel initializes");
            require (panel.initialize ().ok (), "initialization is idempotent");
            require (panel.select (static_cast<adk::SimulatedLoad> (255)).error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid selection is rejected");
            require (panel.select (adk::SimulatedLoad::Heater).ok (),
                     "heater selected before destruction");
        }

        requireAllOff (fan, pump, heater, "destruction turns every output off");
        require       (!fan.initialized () && !pump.initialized () && !heater.initialized (),
                 "destruction releases every output");
    }

    void testIndicatorPumpOwnsOneSafeActiveHighPin ()
    {
        adk::test::arduino::reset ();

        adk::ResourceRegistry resources;
        adk::IndicatorPump    pump (resources, 38);

        require (pump.state () == adk::PumpState::Off, "indicator construction is off");
        require (pump.setState (adk::PumpState::On).error () ==
                     adk::StatusCode::NotInitialized,
                 "indicator rejects commands before initialization");
        require (pump.initialize ().ok (), "indicator initializes");
        require (adk::test::arduino::mode (38) == OUTPUT,
                 "indicator initialization configures output");
        require (adk::test::arduino::digitalOutput (38) == LOW,
                 "indicator initialization establishes low");
        require (pump.setState (adk::PumpState::On).ok (), "indicator turns on");
        require (adk::test::arduino::digitalOutput (38) == HIGH, "on is active high");

        pump.shutdown ();
        pump.shutdown ();

        require (adk::test::arduino::mode (38) == INPUT,
                 "shutdown returns indicator to high impedance");
        require (!resources.claimed ({adk::ResourceKind::Pin, 38}),
                 "shutdown releases indicator pin");
        require (pump.state () == adk::PumpState::Off, "shutdown records semantic off");
    }

    void testIndicatorPumpReportsInvalidAndBusyPins ()
    {
        adk::test::arduino::reset ();

        adk::ResourceRegistry resources;
        adk::IndicatorPump    first   (resources, 38);
        adk::IndicatorPump    busy    (resources, 38);
        adk::IndicatorPump    invalid (resources, 70);

        require (first.initialize ().ok (), "first indicator claims pin");
        require (busy.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "busy indicator pin is rejected");
        require (invalid.initialize ().error () == adk::StatusCode::InvalidPin,
                 "invalid indicator pin is rejected");
    }

    void testPanelPumpOutputMapsOnlyPumpIntent ()
    {
        RecordingOutput     fan;
        RecordingOutput     pump;
        RecordingOutput     heater;
        adk::InertLoadPanel  panel  (fan, pump, heater);
        adk::PanelPumpOutput output (panel);

        require (output.initialize ().error () == adk::StatusCode::NotInitialized,
                 "adapter requires an initialized borrowed panel");
        require (panel.initialize ().ok (), "panel initializes");
        require (output.initialize ().ok (), "adapter initializes");
        require (output.initialize ().ok (), "adapter initialization is idempotent");
        require (output.setState (adk::PumpState::On).ok (),
                 "on selects only the pump");
        require (panel.snapshot ().active == adk::SimulatedLoad::Pump,
                 "pump is the active simulated load");
        require (output.state () == adk::PumpState::On,
                 "adapter reports selected pump");
        require (output.setState (adk::PumpState::Off).ok (),
                 "off selects no load");
        require (panel.snapshot ().active == adk::SimulatedLoad::None,
                 "off clears the panel");
        require (output.setState (static_cast<adk::PumpState> (255)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "adapter rejects an invalid pump state");

        output.shutdown ();
        output.shutdown ();

        require (!output.initialized (), "adapter shutdown is idempotent");
        require (panel.initialized (), "adapter does not own borrowed panel storage");
        require (panel.snapshot ().active == adk::SimulatedLoad::None,
                 "adapter shutdown leaves panel off");
    }

    void testPanelPumpOutputPropagatesPanelFailure ()
    {
        RecordingOutput     fan;
        RecordingOutput     pump;
        RecordingOutput     heater;
        adk::InertLoadPanel  panel  (fan, pump, heater);
        adk::PanelPumpOutput output (panel);

        require (panel.initialize ().ok (), "panel initializes");
        require (output.initialize ().ok (), "adapter initializes");
        pump.failNextSet = true;

        require (output.setState (adk::PumpState::On).error () ==
                     adk::StatusCode::HardwareFailure,
                 "adapter propagates panel output failure");
        require (panel.snapshot ().active == adk::SimulatedLoad::None,
                 "adapter failure leaves every load off");
        require (output.state () == adk::PumpState::Off,
                 "adapter failure reports off");
    }
} // namespace

int main ()
{
    testTransactionalInitialization             ();
    testPreinitializedBorrowedOutputIsUntouched ();
    testEveryTransitionIsExclusive              ();
    testUnchangedSelectionDoesNotWrite          ();
    testTransitionFailuresMakeAllOff            ();
    testLifecycleAndInvalidSelection            ();
    testIndicatorPumpOwnsOneSafeActiveHighPin   ();
    testIndicatorPumpReportsInvalidAndBusyPins  ();
    testPanelPumpOutputMapsOnlyPumpIntent       ();
    testPanelPumpOutputPropagatesPanelFailure   ();
}
