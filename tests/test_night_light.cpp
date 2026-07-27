#include "night_light.h"

#include <cstdlib>
#include <iostream>

namespace {

    using namespace adk;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    NightLightConfig testConfig ()
    {
        NightLightConfig config;

        config.turnOnBelowPermille  = 300;
        config.turnOffAbovePermille = 500;
        config.minimumDuty          = 20;
        config.maximumDuty          = 220;

        return config;
    }

    void requireSafeFault (NightLight&          nightLight,
                           LightSampleState     sampleState,
                           const char*          message)
    {
        require (nightLight.update (NightLightInput (400, sampleState)) ==
                 Status::HardwareFailure,
                 message);

        const NightLightSnapshot snapshot = nightLight.snapshot ();

        require (snapshot.mode == NightLightMode::Fault, message);
        require (snapshot.sampleState == sampleState, message);
        require (snapshot.diagnostic == NightLightDiagnostic::SensorFault, message);
        require (snapshot.outputDuty == 0, message);
        require (!snapshot.lampOn, message);
    }

    void testLifecycleAndConfiguration ()
    {
        NightLightConfig config     = testConfig ();
        NightLight       nightLight = NightLight (config);

        require (nightLight.update (NightLightInput ()) == Status::NotInitialized,
                 "update before initialize");
        require (nightLight.initialize () == Status::Ok, "initialize");
        require (nightLight.initialize () == Status::Ok, "repeat initialize");

        const NightLightSnapshot snapshot = nightLight.snapshot ();

        require (snapshot.mode == NightLightMode::Off, "initialize off");
        require (snapshot.outputDuty == 0, "initialize safe duty");
        require (snapshot.diagnostic == NightLightDiagnostic::Ready,
                 "initialize diagnostic");

        config.turnOnBelowPermille = config.turnOffAbovePermille;
        NightLight invalidThresholds (config);

        require (invalidThresholds.initialize () == Status::InvalidArgument,
                 "reject equal thresholds");

        config                         = testConfig ();
        config.turnOffAbovePermille    = 1001;
        NightLight invalidRange (config);

        require (invalidRange.initialize () == Status::InvalidArgument,
                 "reject threshold range");

        config                     = testConfig ();
        config.minimumDuty         = 0;
        NightLight invisibleOutput (config);

        require (invisibleOutput.initialize () == Status::InvalidArgument,
                 "reject invisible diagnostic output");

        config                     = testConfig ();
        config.minimumDuty         = 221;
        NightLight reversedDuty (config);

        require (reversedDuty.initialize () == Status::InvalidArgument,
                 "reject reversed duty");
    }

    void testHysteresisAndBrightness ()
    {
        NightLight nightLight (testConfig ());

        require (nightLight.initialize () == Status::Ok, "hysteresis initialize");
        require (nightLight.update (NightLightInput (300)) == Status::Ok,
                 "on threshold remains off");
        require (!nightLight.snapshot ().lampOn, "strict on threshold");

        require (nightLight.update (NightLightInput (299)) == Status::Ok,
                 "dark sample");

        const uint8_t firstDuty = nightLight.snapshot ().outputDuty;

        require (nightLight.snapshot ().lampOn, "dark turns lamp on");
        require (firstDuty > 20 && firstDuty < 220, "scaled initial duty");

        require (nightLight.update (NightLightInput (450)) == Status::Ok,
                 "hysteresis band rising");
        require (nightLight.snapshot ().lampOn, "lamp stays on in band");
        require (nightLight.snapshot ().outputDuty < firstDuty,
                 "brighter sample lowers duty");

        require (nightLight.update (NightLightInput (500)) == Status::Ok,
                 "off threshold remains on");
        require (nightLight.snapshot ().lampOn, "strict off threshold");
        require (nightLight.snapshot ().outputDuty == 20,
                 "minimum visible duty at boundary");

        require (nightLight.update (NightLightInput (501)) == Status::Ok,
                 "bright sample");
        require (!nightLight.snapshot ().lampOn, "bright turns lamp off");
        require (nightLight.snapshot ().outputDuty == 0, "off has zero duty");

        require (nightLight.update (NightLightInput (400)) == Status::Ok,
                 "hysteresis band falling");
        require (!nightLight.snapshot ().lampOn, "lamp stays off in band");

        require (nightLight.update (NightLightInput (0)) == Status::Ok,
                 "darkest sample");
        require (nightLight.snapshot ().outputDuty == 220, "bounded maximum duty");
    }

    void testBrightnessProperties ()
    {
        NightLight nightLight (testConfig ());

        require (nightLight.initialize () == Status::Ok, "property initialize");

        uint8_t previousDuty = 220;

        for (uint16_t lightPermille = 0; lightPermille <= 500; ++lightPermille)
        {
            require (nightLight.update (NightLightInput (lightPermille)) == Status::Ok,
                     "property update");

            const NightLightSnapshot snapshot = nightLight.snapshot ();

            require (snapshot.lampOn, "active through strict off threshold");
            require (snapshot.outputDuty >= 20, "duty has configured lower bound");
            require (snapshot.outputDuty <= 220, "duty has configured upper bound");
            require (snapshot.outputDuty <= previousDuty,
                     "duty is monotonic as measured light rises");

            previousDuty = snapshot.outputDuty;
        }

        require (nightLight.snapshot ().outputDuty == 20,
                 "off threshold reaches minimum duty");
        require (nightLight.update (NightLightInput (501)) == Status::Ok,
                 "first sample beyond off threshold");
        require (nightLight.snapshot ().outputDuty == 0,
                 "inactive duty remains outside active range");
    }

    void testBoundaryConfiguration ()
    {
        NightLightConfig config;

        config.turnOnBelowPermille  = 1;
        config.turnOffAbovePermille = 1000;
        config.minimumDuty          = 1;
        config.maximumDuty          = 255;

        NightLight widestValidRange (config);

        require (widestValidRange.initialize () == Status::Ok,
                 "accept widest threshold and duty ranges");
        require (widestValidRange.update (NightLightInput (0)) == Status::Ok,
                 "darkest sample");
        require (widestValidRange.snapshot ().lampOn,
                 "darkest sample activates widest useful range");
        require (widestValidRange.snapshot ().outputDuty == 255,
                 "darkest sample reaches maximum duty");

        config.turnOnBelowPermille  = 999;
        config.turnOffAbovePermille = 1000;
        config.minimumDuty          = 255;
        config.maximumDuty          = 255;

        NightLight narrowValidRange (config);

        require (narrowValidRange.initialize () == Status::Ok,
                 "accept adjacent thresholds and fixed duty");
        require (narrowValidRange.update (NightLightInput (998)) == Status::Ok,
                 "activate narrow range");
        require (narrowValidRange.snapshot ().outputDuty == 255,
                 "fixed duty remains fixed");
    }

    void testReinitializeResetsState ()
    {
        NightLight nightLight (testConfig ());

        require (nightLight.initialize () == Status::Ok, "reset initialize");
        require (nightLight.update (NightLightInput (0)) == Status::Ok,
                 "reset active precondition");
        require (nightLight.snapshot ().lampOn, "reset active state");
        require (nightLight.initialize () == Status::Ok, "reset while active");
        require (!nightLight.snapshot ().lampOn, "reinitialize turns output off");
        require (nightLight.snapshot ().outputDuty == 0, "reinitialize clears duty");

        require (nightLight.update (
                     NightLightInput (0, LightSampleState::Stale)) ==
                 Status::HardwareFailure,
                 "reset fault precondition");
        require (nightLight.initialize () == Status::Ok, "reset while faulted");

        const NightLightSnapshot snapshot = nightLight.snapshot ();

        require (snapshot.mode == NightLightMode::Off, "reset clears fault mode");
        require (snapshot.sampleState == LightSampleState::Valid,
                 "reset clears fault sample state");
        require (snapshot.status == Status::Ok, "reset clears fault status");
        require (snapshot.diagnostic == NightLightDiagnostic::Ready,
                 "reset restores ready diagnostic");
    }

    void testFaultsAndRecovery ()
    {
        NightLight nightLight (testConfig ());

        require (nightLight.initialize () == Status::Ok, "fault initialize");
        require (nightLight.update (NightLightInput (0)) == Status::Ok,
                 "fault precondition");
        require (nightLight.snapshot ().lampOn, "fault precondition active");

        requireSafeFault (nightLight, LightSampleState::BelowRange, "below-range fault");
        requireSafeFault (nightLight, LightSampleState::AboveRange, "above-range fault");
        requireSafeFault (nightLight, LightSampleState::Stale, "stale fault");

        require (nightLight.update (NightLightInput (250)) == Status::Ok,
                 "valid sample recovers");
        require (nightLight.snapshot ().mode == NightLightMode::On,
                 "recovery reevaluates threshold");

        require (nightLight.update (NightLightInput (1001)) == Status::InvalidArgument,
                 "reject invalid normalized sample");
        require (nightLight.snapshot ().mode == NightLightMode::Fault,
                 "invalid sample faults");
        require (nightLight.snapshot ().outputDuty == 0,
                 "invalid sample safe output");
    }

    void testDeterministicReplay ()
    {
        const NightLightInput trace[] = {
            NightLightInput (700),
            NightLightInput (250),
            NightLightInput (400),
            NightLightInput (600),
            NightLightInput (0, LightSampleState::BelowRange),
            NightLightInput (200)};
        NightLight first  (testConfig ());
        NightLight second (testConfig ());

        require (first.initialize () == Status::Ok, "first replay initialize");
        require (second.initialize () == Status::Ok, "second replay initialize");

        for (const NightLightInput& input : trace)
        {
            require (first.update (input) == second.update (input), "replay status");

            const NightLightSnapshot left  = first.snapshot  ();
            const NightLightSnapshot right = second.snapshot ();

            require (left.mode == right.mode, "replay mode");
            require (left.sampleState == right.sampleState, "replay sample state");
            require (left.diagnostic == right.diagnostic, "replay diagnostic");
            require (left.status == right.status, "replay snapshot status");
            require (left.lightPermille == right.lightPermille, "replay light");
            require (left.outputDuty == right.outputDuty, "replay duty");
            require (left.lampOn == right.lampOn, "replay lamp");
        }
    }
}

int main ()
{
    testLifecycleAndConfiguration  ();
    testHysteresisAndBrightness    ();
    testBrightnessProperties       ();
    testBoundaryConfiguration      ();
    testReinitializeResetsState    ();
    testFaultsAndRecovery          ();
    testDeterministicReplay        ();

    std::cout << "night light tests passed\n";
    return 0;
}
