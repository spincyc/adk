#include <servo_output.h>

#include <cstdlib>
#include <iostream>
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

    struct TestPower final : adk::PowerDomain
    {
        bool commandAdmitted () const noexcept override
        {
            return available_;
        }

        bool available_ = false;
    };

    struct IoOperation
    {
        enum struct Kind
        {
            Attach,
            Write,
            Detach
        };

        Kind     kind;
        uint8_t  pin;
        uint16_t value;
    };

    struct TestIo final : adk::ServoPulseIo
    {
        adk::Status attach (adk::PinId pin, uint8_t timer) noexcept override
        {
            operations.push_back ({IoOperation::Kind::Attach, pin, timer});
            return attachStatus;
        }

        adk::Status writePulse (
            adk::PinId pin,
            uint16_t   pulseUs) noexcept override
        {
            operations.push_back ({IoOperation::Kind::Write, pin, pulseUs});
            return writeStatus;
        }

        void detach (adk::PinId pin) noexcept override
        {
            operations.push_back ({IoOperation::Kind::Detach, pin, 0});
        }

        adk::Status             attachStatus = adk::StatusCode::Ok;
        adk::Status             writeStatus  = adk::StatusCode::Ok;
        std::vector<IoOperation> operations;
    };

    struct TestRegisters final : adk::MegaTimer5Registers
    {
        adk::Status writeControlA (uint8_t value) noexcept override
        {
            if (failConnect && value == 10)
            {
                failConnect = false;
                return adk::StatusCode::HardwareFailure;
            }

            controlA = value;
            return status;
        }

        adk::Status writeControlB (uint8_t value) noexcept override
        {
            controlB = value;
            return status;
        }

        adk::Status writeControlC (uint8_t value) noexcept override
        {
            controlC = value;
            return status;
        }

        adk::Status writeInterruptMask (uint8_t value) noexcept override
        {
            interruptMask = value;
            return status;
        }

        adk::Status writeCounter (uint16_t value) noexcept override
        {
            counter = value;
            return status;
        }

        adk::Status writeTop (uint16_t value) noexcept override
        {
            top = value;
            return failTop ? adk::StatusCode::HardwareFailure : status;
        }

        adk::Status writeCompareC (uint16_t value) noexcept override
        {
            if (failCompare && value != 0)
            {
                failCompare = false;
                return adk::StatusCode::HardwareFailure;
            }

            compareC = value;
            return status;
        }

        adk::Status writeOutputLow () noexcept override
        {
            outputLow = true;
            return status;
        }

        adk::Status setOutputEnabled (bool enabled) noexcept override
        {
            outputEnabled = enabled;
            return status;
        }

        adk::Status status        = adk::StatusCode::Ok;
        uint8_t     controlA      = 0;
        uint8_t     controlB      = 0;
        uint8_t     controlC      = 0;
        uint8_t     interruptMask = 0;
        uint16_t    counter       = 0;
        uint16_t    top           = 0;
        uint16_t    compareC      = 0;
        bool        outputLow     = false;
        bool        outputEnabled = false;
        bool        failTop       = false;
        bool        failCompare   = false;
        bool        failConnect   = false;
    };

    void testLifecycleAndBounds ()
    {
        adk::ResourceRegistry resources;
        TestIo                io;
        TestPower             power;
        adk::ServoOutput      servo (resources, io, power, 9, 1000, 2000);

        require (servo.pulseUs () == 0, "construction is inert");
        require (servo.writePulse (1500).error () ==
                     adk::StatusCode::NotInitialized,
                 "write before initialization");
        require (servo.initialize ().ok (), "servo initializes");
        require (servo.initialize ().ok (), "initialization is idempotent");
        require (io.operations.size () == 1, "attach occurs once");
        require (servo.writePulse (999).error () ==
                     adk::StatusCode::InvalidArgument,
                 "pulse below configured bound");
        require (io.operations.size () == 1, "bad pulse is inert");
        require (servo.writePulse (1500).error () ==
                     adk::StatusCode::HardwareFailure,
                 "unavailable power rejects motion");

        power.available_ = true;
        require (servo.writePulse (1000).ok (), "minimum pulse");
        require (servo.writePulse (2000).ok (), "maximum pulse");
        require (servo.pulseUs () == 2000, "last pulse snapshot");

        servo.shutdown ();
        servo.shutdown ();

        require (!servo.initialized (), "shutdown clears state");
        require (servo.pulseUs () == 0, "shutdown clears pulse");
        require (io.operations.back ().kind == IoOperation::Kind::Detach,
                 "shutdown detaches signal");
        require (!resources.claimed ({adk::ResourceKind::Pin, 9}),
                 "shutdown releases pin");
        require (!resources.claimed ({adk::ResourceKind::Timer, 5}),
                 "shutdown releases timer");
    }

    void testValidationAndRollback ()
    {
        adk::ResourceRegistry resources;
        TestIo                io;
        TestPower             power;

        adk::ServoOutput badBounds (resources, io, power, 9, 500, 2000);

        require (badBounds.initialize ().error () ==
                     adk::StatusCode::InvalidArgument,
                 "absolute pulse bounds enforced");
        require (io.operations.empty (), "bad configuration is inert");

        adk::ResourceClaim blocker;
        require (resources.claim ({adk::ResourceKind::Timer, 5}, blocker).ok (),
                 "timer blocker claims");

        adk::ServoOutput busy (resources, io, power, 9, 1000, 2000);

        require (busy.initialize ().error () ==
                     adk::StatusCode::ResourceBusy,
                 "timer conflict reported");
        require (!resources.claimed ({adk::ResourceKind::Pin, 9}),
                 "timer conflict rolls back pin");

        blocker.release ();
        io.attachStatus = adk::StatusCode::HardwareFailure;

        require (busy.initialize ().error () ==
                     adk::StatusCode::HardwareFailure,
                 "attach failure propagated");
        require (!resources.claimed ({adk::ResourceKind::Pin, 9}),
                 "attach failure releases pin");
        require (!resources.claimed ({adk::ResourceKind::Timer, 5}),
                 "attach failure releases timer");
    }

    void testWriteFailureAndDestruction ()
    {
        adk::ResourceRegistry resources;
        TestIo                io;
        TestPower             power;
        power.available_ = true;

        {
            adk::ServoOutput servo (resources, io, power, 9, 1000, 2000);

            require (servo.initialize ().ok (), "scoped servo initializes");

            io.writeStatus = adk::StatusCode::HardwareFailure;
            require (servo.writePulse (1500).error () ==
                         adk::StatusCode::HardwareFailure,
                     "write failure propagated");
            require (servo.pulseUs () == 0, "failed write preserves snapshot");
        }

        require (io.operations.back ().kind == IoOperation::Kind::Detach,
                 "destructor detaches");

        adk::ServoOutput replacement (resources, io, power, 9, 1000, 2000);
        io.attachStatus = adk::StatusCode::Ok;
        require (replacement.initialize ().ok (), "resources reusable");
    }

    void testMegaTimer5Adapter ()
    {
        TestRegisters                   registers;
        adk::MegaTimer5ServoPulseIo     io (registers);

        require (io.attach (9, 5).error () == adk::StatusCode::Unsupported,
                 "adapter requires D44");
        require (io.attach (44, 4).error () == adk::StatusCode::Unsupported,
                 "adapter requires Timer5");
        require (io.attach (44, 5).ok (), "adapter configures Timer5");
        require (registers.top == 39999, "adapter selects 20 ms period");
        require (registers.controlA == 2, "compare remains disconnected");
        require (registers.controlB == 26, "fast PWM mode and divide-by-eight");
        require (registers.outputEnabled, "D44 output enabled");

        require (io.writePulse (44, 1500).ok (), "adapter writes pulse");
        require (registers.compareC == 3000, "microseconds convert to timer ticks");
        require (registers.controlA == 10, "OC5C compare output connected");

        io.detach (44);

        require (registers.controlA == 0 && registers.controlB == 0,
                 "detach stops Timer5");
        require (registers.compareC == 0 && registers.top == 0,
                 "detach clears pulse registers");
        require (!registers.outputEnabled && registers.outputLow,
                 "detach leaves D44 low then high impedance");

        registers.failTop = true;
        require (io.attach (44, 5).error () ==
                     adk::StatusCode::HardwareFailure,
                 "register failure propagated");
        require (!registers.outputEnabled && registers.controlB == 0,
                 "partial attach rolls back to safe state");
    }

    void testMegaTimer5WriteFailure (bool compareFailure)
    {
        adk::ResourceRegistry          resources;
        TestRegisters                  registers;
        adk::MegaTimer5ServoPulseIo    io (registers);
        TestPower                      power;
        adk::ServoOutput               servo (
            resources,
            io,
            power,
            adk::MegaTimer5ServoPulseIo::signalPin,
            1000,
            2000,
            adk::MegaTimer5ServoPulseIo::timer);

        power.available_ = true;
        require (servo.initialize ().ok (), "failure fixture initializes");
        require (servo.writePulse (1200).ok (), "failure fixture activates");
        require (servo.pulseUs () == 1200, "active pulse recorded");

        registers.failCompare = compareFailure;
        registers.failConnect = !compareFailure;

        require (servo.writePulse (1600).error () ==
                     adk::StatusCode::HardwareFailure,
                 "write failure propagated");
        require (servo.pulseUs () == 0, "write failure clears pulse snapshot");
        require (registers.controlA == 0 && registers.controlB == 0,
                 "write failure stops Timer5");
        require (registers.compareC == 0 && registers.top == 0,
                 "write failure clears waveform");
        require (!registers.outputEnabled && registers.outputLow,
                 "write failure leaves D44 low then high impedance");
        require (servo.writePulse (1500).error () ==
                     adk::StatusCode::NotInitialized,
                 "faulted adapter rejects another pulse");

        servo.shutdown ();

        require (!resources.claimed ({adk::ResourceKind::Pin, 44}),
                 "fault shutdown releases D44");
        require (!resources.claimed ({adk::ResourceKind::Timer, 5}),
                 "fault shutdown releases Timer5");
        require (servo.initialize ().ok (), "faulted endpoint reinitializes");
        require (servo.writePulse (1500).ok (), "reinitialized endpoint pulses");
    }
}

int main ()
{
    testLifecycleAndBounds          ();
    testValidationAndRollback       ();
    testWriteFailureAndDestruction  ();
    testMegaTimer5Adapter           ();
    testMegaTimer5WriteFailure      (true);
    testMegaTimer5WriteFailure      (false);

    std::cout << "All ADK servo-output tests passed.\n";
    return EXIT_SUCCESS;
}
