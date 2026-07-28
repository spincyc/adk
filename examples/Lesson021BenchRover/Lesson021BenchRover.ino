// Mega 2560, USB logic only: D22-D27 each drive an LED through 1 kOhm.
// The fixed show ends automatically. Do not connect motors, a driver, or load power.
#include <Adk.h>
#include <motor_intent.h>
#include <power_domain.h>
#include <rover_controller.h>
#include <ultrasonic_ranger.h>

namespace {

    constexpr adk::PinId leftForwardPin  = 22;
    constexpr adk::PinId leftReversePin  = 23;
    constexpr adk::PinId leftEnablePin   = 24;
    constexpr adk::PinId rightForwardPin = 25;
    constexpr adk::PinId rightReversePin = 26;
    constexpr adk::PinId rightEnablePin  = 27;
    constexpr uint32_t   showDurationMs  = 12000;

    const adk::UltrasonicRangerConfig rangeConfig = {adk::MicrosecondDuration (30000),
                                                     adk::MicrosecondDuration (30000),
                                                     20, 4000, 343};

    adk::RoverControllerConfig makeRoverConfig ()
    {
        adk::RoverControllerConfig config;

        config.minimumClearanceMm         = 200;
        config.resumeClearanceMm          = 250;
        config.rangeMaximumAge            = adk::Duration (100);
        config.motionStartTimeout         = adk::Duration (500);
        config.wheelMismatchTimeout       = adk::Duration (500);
        config.maximumWheelEdgeDifference = 3;
        config.maximumMotorDuty           = 160;

        return config;
    }

    const adk::RoverControllerConfig roverConfig = makeRoverConfig ();

    const adk::RouteStep route[] = {
        adk::RouteStep (adk::RouteAction::Drive, 8, adk::Duration (3000), 96),
        adk::RouteStep (adk::RouteAction::TurnLeft, 4, adk::Duration (2000), 80),
        adk::RouteStep (adk::RouteAction::Pause, 0, adk::Duration (500), 0),
        adk::RouteStep (adk::RouteAction::Drive, 8, adk::Duration (3000), 96),
        adk::RouteStep (adk::RouteAction::Finish)};

    adk::Runtime runtime;

    adk::MonoLed leftForwardEvidence  (runtime.resources (), leftForwardPin);
    adk::MonoLed leftReverseEvidence  (runtime.resources (), leftReversePin);
    adk::MonoLed leftEnableEvidence   (runtime.resources (), leftEnablePin);
    adk::MonoLed rightForwardEvidence (runtime.resources (), rightForwardPin);
    adk::MonoLed rightReverseEvidence (runtime.resources (), rightReversePin);
    adk::MonoLed rightEnableEvidence  (runtime.resources (), rightEnablePin);
    adk::MonoLed acquisitionEvidence  (runtime.resources (), LED_BUILTIN);

    adk::ExternalPowerDomainGate commandGate;
    adk::MotorIntent      leftMotor (adk::MotorIntentConfig (adk::Duration (100), 160),
                                     commandGate);
    adk::MotorIntent      rightMotor (adk::MotorIntentConfig (adk::Duration (100), 160),
                                      commandGate);
    adk::UltrasonicRanger ranger (rangeConfig);
    adk::RoverController  rover  (roverConfig, route, sizeof (route) / sizeof (route[0]));

    enum struct EchoPhase : uint8_t
    {
        Idle,
        Waiting,
        High
    };

    EchoPhase                    echoPhase = EchoPhase::Idle;
    adk::MicrosecondTimePoint    echoChangedAt;
    adk::MicrosecondTimePoint    rangeObservedAtUs;
    adk::TimePoint               simulationStartedAt;
    adk::TimePoint               previousControlAt;
    adk::RangeReading            observedRange = {adk::RangeState::Idle, 0,
                                                  adk::MicrosecondDuration (0), false};
    adk::RoverControllerSnapshot decision;
    uint32_t                     leftEdges          = 0;
    uint32_t                     rightEdges         = 0;
    uint32_t                     leftEdgeRemainder  = 0;
    uint32_t                     rightEdgeRemainder = 0;
    bool                         routeStarted       = false;
    bool                         running            = false;

    bool initializeRoverSimulation ();

    bool observeSimulatedRange  (adk::MicrosecondTimePoint now);
    void observeSimulatedWheels (adk::TimePoint now);
    bool decideRoverMotion      (adk::TimePoint now);
    bool actuateMotorEvidence   (adk::TimePoint now);

    uint32_t simulatedEchoDuration (adk::MicrosecondTimePoint now);
    void     advanceWheel          (const adk::MotorCommand& command, uint32_t& edges,
                                    uint32_t& remainder);
    bool     commandMotor          (adk::MotorIntent& motor,
                                    const adk::MotorCommand& command,
                                    adk::TimePoint now);
    bool     showMotorEvidence     (const adk::MotorCommand& command,
                                    adk::MonoLed& forward,
                                    adk::MonoLed& reverse,
                                    adk::MonoLed& enable);
    bool     showSafeState         ();
    void     stopSafely            ();

} // namespace

void setup ()
{
    running = initializeRoverSimulation ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::MicrosecondTimePoint nowUs (micros ());
    const adk::TimePoint            now   (millis ());

    if (now.elapsedSince (simulationStartedAt).milliseconds () >= showDurationMs)
    {
        stopSafely ();
        return;
    }

    if (!observeSimulatedRange (nowUs))
    {
        stopSafely ();
        return;
    }

    if (now.elapsedSince (previousControlAt).milliseconds () < 20)
    {
        return;
    }

    observeSimulatedWheels (now);

    if (!decideRoverMotion (now) || !actuateMotorEvidence (now))
    {
        stopSafely ();
    }
}

namespace {

    bool initializeRoverSimulation ()
    {
        if (!leftForwardEvidence.initialize  ().ok () ||
            !leftReverseEvidence.initialize  ().ok () ||
            !leftEnableEvidence.initialize   ().ok () ||
            !rightForwardEvidence.initialize ().ok () ||
            !rightReverseEvidence.initialize ().ok () ||
            !rightEnableEvidence.initialize  ().ok () ||
            !acquisitionEvidence.initialize  ().ok () ||
            !leftMotor.initialize            ().ok () ||
            !rightMotor.initialize           ().ok () ||
            !ranger.initialize               ().ok () ||
            !rover.initialize                ().ok ())
        {
            stopSafely ();
            return false;
        }

        commandGate.admit ();

        simulationStartedAt = adk::TimePoint (millis ());
        previousControlAt   = simulationStartedAt;

        if (!showSafeState () || !acquisitionEvidence.on ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    bool observeSimulatedRange (adk::MicrosecondTimePoint now)
    {
        if (echoPhase == EchoPhase::Idle)
        {
            if (!ranger.startMeasurement (now, false).ok ())
            {
                return false;
            }

            echoPhase     = EchoPhase::Waiting;
            echoChangedAt = now;
            return true;
        }

        const uint32_t elapsed = now.elapsedSince (echoChangedAt).microseconds ();

        if (echoPhase == EchoPhase::Waiting && elapsed >= 100)
        {
            const adk::MicrosecondTimePoint echoStartedAt (
                echoChangedAt.microseconds () + 100U);

            echoPhase     = EchoPhase::High;
            echoChangedAt = echoStartedAt;
            return ranger.update (echoStartedAt, true).ok ();
        }

        const uint32_t echoDuration = simulatedEchoDuration (now);

        if (echoPhase == EchoPhase::High && elapsed >= echoDuration)
        {
            const adk::MicrosecondTimePoint echoEndedAt (echoChangedAt.microseconds () +
                                                         echoDuration);

            if (!ranger.update (echoEndedAt, false).ok ())
            {
                return false;
            }

            rangeObservedAtUs = echoEndedAt;
            observedRange     = ranger.reading ();
            echoPhase         = EchoPhase::Idle;
        }

        return true;
    }

    void observeSimulatedWheels (adk::TimePoint now)
    {
        advanceWheel (decision.leftMotor, leftEdges, leftEdgeRemainder);
        advanceWheel (decision.rightMotor, rightEdges, rightEdgeRemainder);

        previousControlAt = now;
    }

    bool decideRoverMotion (adk::TimePoint now)
    {
        const bool startEvent = !routeStarted && observedRange.valid;

        const adk::RoverInput input (
            observedRange,
            adk::TimePoint             (rangeObservedAtUs.microseconds () / 1000U),
            adk::RoverWheelObservation (leftEdges),
            adk::RoverWheelObservation (rightEdges),
            startEvent,
            false,
            now);

        routeStarted = routeStarted || startEvent;

        if (!rover.update (input).ok ())
        {
            return false;
        }

        decision = rover.snapshot ();
        return decision.status.ok ();
    }

    bool actuateMotorEvidence (adk::TimePoint now)
    {
        if (!commandMotor (leftMotor, decision.leftMotor, now) ||
            !commandMotor (rightMotor, decision.rightMotor, now))
        {
            return false;
        }

        return showMotorEvidence (leftMotor.snapshot ().applied, leftForwardEvidence,
                                  leftReverseEvidence, leftEnableEvidence) &&
               showMotorEvidence (rightMotor.snapshot ().applied, rightForwardEvidence,
                                  rightReverseEvidence, rightEnableEvidence);
    }

    uint32_t simulatedEchoDuration (adk::MicrosecondTimePoint now)
    {
        const adk::TimePoint observedAt (now.microseconds () / 1000U);
        const uint32_t       elapsed =
            observedAt.elapsedSince (simulationStartedAt).milliseconds ();
        const uint16_t distanceMm = elapsed >= 2500U && elapsed < 3500U ? 150 : 500;

        return static_cast<uint32_t> (distanceMm) * 2000U / 343U;
    }

    void advanceWheel (const adk::MotorCommand& command, uint32_t& edges,
                       uint32_t& remainder)
    {
        if (command.direction == adk::MotorDirection::Stopped)
        {
            return;
        }

        remainder += command.duty;

        while (remainder >= 1600)
        {
            remainder -= 1600;
            ++edges;
        }
    }

    bool commandMotor (adk::MotorIntent& motor, const adk::MotorCommand& command,
                       adk::TimePoint now)
    {
        return motor.command (command, now).ok () && motor.update (now).ok ();
    }

    bool showMotorEvidence (const adk::MotorCommand& command, adk::MonoLed& forward,
                            adk::MonoLed& reverse, adk::MonoLed& enable)
    {
        const bool forwardOn = command.direction == adk::MotorDirection::Forward;
        const bool reverseOn = command.direction == adk::MotorDirection::Reverse;
        const bool enableOn  = command.duty > 0;

        return forward.set (forwardOn).ok () && reverse.set (reverseOn).ok () &&
               enable.set (enableOn).ok ();
    }

    bool showSafeState ()
    {
        return showMotorEvidence (adk::MotorCommand (), leftForwardEvidence,
                                  leftReverseEvidence, leftEnableEvidence) &&
               showMotorEvidence (adk::MotorCommand (), rightForwardEvidence,
                                  rightReverseEvidence, rightEnableEvidence);
    }

    void stopSafely ()
    {
        rover.shutdown         ();
        leftMotor.stop         ();
        rightMotor.stop        ();
        showSafeState          ();
        commandGate.revoke     ();
        ranger.reset           ();
        rightMotor.shutdown    ();
        leftMotor.shutdown     ();

        acquisitionEvidence.shutdown  ();
        rightEnableEvidence.shutdown  ();
        rightReverseEvidence.shutdown ();
        rightForwardEvidence.shutdown ();
        leftEnableEvidence.shutdown   ();
        leftReverseEvidence.shutdown  ();
        leftForwardEvidence.shutdown  ();

        running = false;
    }

} // namespace
