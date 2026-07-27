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

    void requireOk (Status status, const char* message)
    {
        require (status.ok (), message);
    }

    void requireError (Status status, StatusCode code, const char* message)
    {
        require (status.error () == code, message);
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
        requireError (nightLight.update (NightLightInput (400, sampleState)),
                      StatusCode::HardwareFailure,
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

        requireError (nightLight.update (NightLightInput ()),
                      StatusCode::NotInitialized,
                      "update before initialize");
        requireOk (nightLight.initialize (), "initialize");
        requireOk (nightLight.initialize (), "repeat initialize");

        const NightLightSnapshot snapshot = nightLight.snapshot ();

        require (snapshot.mode == NightLightMode::Off, "initialize off");
        require (snapshot.outputDuty == 0, "initialize safe duty");
        require (snapshot.diagnostic == NightLightDiagnostic::Ready,
                 "initialize diagnostic");

        config.turnOnBelowPermille = config.turnOffAbovePermille;
        NightLight invalidThresholds (config);

        requireError (invalidThresholds.initialize (),
                      StatusCode::InvalidArgument,
                      "reject equal thresholds");

        config                         = testConfig ();
        config.turnOffAbovePermille    = 1001;
        NightLight invalidRange (config);

        requireError (invalidRange.initialize (),
                      StatusCode::InvalidArgument,
                      "reject threshold range");

        config                     = testConfig ();
        config.minimumDuty         = 0;
        NightLight invisibleOutput (config);

        requireError (invisibleOutput.initialize (),
                      StatusCode::InvalidArgument,
                      "reject invisible diagnostic output");

        config                     = testConfig ();
        config.minimumDuty         = 221;
        NightLight reversedDuty (config);

        requireError (reversedDuty.initialize (),
                      StatusCode::InvalidArgument,
                      "reject reversed duty");
    }

    void testHysteresisAndBrightness ()
    {
        NightLight nightLight (testConfig ());

        requireOk (nightLight.initialize (), "hysteresis initialize");
        requireOk (nightLight.update (NightLightInput (300)),
                   "on threshold remains off");
        require (!nightLight.snapshot ().lampOn, "strict on threshold");

        requireOk (nightLight.update (NightLightInput (299)), "dark sample");

        const uint8_t firstDuty = nightLight.snapshot ().outputDuty;

        require (nightLight.snapshot ().lampOn, "dark turns lamp on");
        require (firstDuty > 20 && firstDuty < 220, "scaled initial duty");

        requireOk (nightLight.update (NightLightInput (450)),
                   "hysteresis band rising");
        require (nightLight.snapshot ().lampOn, "lamp stays on in band");
        require (nightLight.snapshot ().outputDuty < firstDuty,
                 "brighter sample lowers duty");

        requireOk (nightLight.update (NightLightInput (500)),
                   "off threshold remains on");
        require (nightLight.snapshot ().lampOn, "strict off threshold");
        require (nightLight.snapshot ().outputDuty == 20,
                 "minimum visible duty at boundary");

        requireOk (nightLight.update (NightLightInput (501)), "bright sample");

        require (!nightLight.snapshot ().lampOn, "bright turns lamp off");
        require (nightLight.snapshot ().outputDuty == 0, "off has zero duty");

        requireOk (nightLight.update (NightLightInput (400)),
                   "hysteresis band falling");
        require (!nightLight.snapshot ().lampOn, "lamp stays off in band");

        requireOk (nightLight.update (NightLightInput (0)), "darkest sample");

        require (nightLight.snapshot ().outputDuty == 220, "bounded maximum duty");
    }

    void testBrightnessProperties ()
    {
        NightLight nightLight (testConfig ());

        requireOk (nightLight.initialize (), "property initialize");

        uint8_t previousDuty = 220;

        for (uint16_t lightPermille = 0; lightPermille <= 500; ++lightPermille)
        {
            requireOk (nightLight.update (NightLightInput (lightPermille)),
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
        requireOk (nightLight.update (NightLightInput (501)),
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

        requireOk (widestValidRange.initialize (),
                   "accept widest threshold and duty ranges");
        requireOk (widestValidRange.update (NightLightInput (0)), "darkest sample");

        require (widestValidRange.snapshot ().lampOn,
                 "darkest sample activates widest useful range");
        require (widestValidRange.snapshot ().outputDuty == 255,
                 "darkest sample reaches maximum duty");

        config.turnOnBelowPermille  = 999;
        config.turnOffAbovePermille = 1000;
        config.minimumDuty          = 255;
        config.maximumDuty          = 255;

        NightLight narrowValidRange (config);

        requireOk (narrowValidRange.initialize (),
                   "accept adjacent thresholds and fixed duty");
        requireOk (narrowValidRange.update (NightLightInput (998)),
                   "activate narrow range");
        require (narrowValidRange.snapshot ().outputDuty == 255,
                 "fixed duty remains fixed");
    }

    void testReinitializeResetsState ()
    {
        NightLight nightLight (testConfig ());

        requireOk (nightLight.initialize (), "reset initialize");
        requireOk (nightLight.update (NightLightInput (0)),
                   "reset active precondition");

        require (nightLight.snapshot ().lampOn, "reset active state");

        requireOk (nightLight.initialize (), "reset while active");

        require (!nightLight.snapshot ().lampOn, "reinitialize turns output off");
        require (nightLight.snapshot ().outputDuty == 0, "reinitialize clears duty");

        requireError (
            nightLight.update (NightLightInput (0, LightSampleState::Stale)),
            StatusCode::HardwareFailure,
            "reset fault precondition");
        requireOk (nightLight.initialize (), "reset while faulted");

        const NightLightSnapshot snapshot = nightLight.snapshot ();

        require (snapshot.mode == NightLightMode::Off, "reset clears fault mode");
        require (snapshot.sampleState == LightSampleState::Valid,
                 "reset clears fault sample state");
        require (snapshot.status.ok (), "reset clears fault status");
        require (snapshot.diagnostic == NightLightDiagnostic::Ready,
                 "reset restores ready diagnostic");
    }

    void testFaultsAndRecovery ()
    {
        NightLight nightLight (testConfig ());

        requireOk (nightLight.initialize (), "fault initialize");
        requireOk (nightLight.update (NightLightInput (0)), "fault precondition");

        require (nightLight.snapshot ().lampOn, "fault precondition active");

        requireSafeFault (nightLight, LightSampleState::BelowRange, "below-range fault");
        requireSafeFault (nightLight, LightSampleState::AboveRange, "above-range fault");
        requireSafeFault (nightLight, LightSampleState::Stale, "stale fault");

        requireOk (nightLight.update (NightLightInput (250)),
                   "valid sample recovers");
        require (nightLight.snapshot ().mode == NightLightMode::On,
                 "recovery reevaluates threshold");

        requireError (nightLight.update (NightLightInput (1001)),
                      StatusCode::InvalidArgument,
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

        requireOk (first.initialize (), "first replay initialize");
        requireOk (second.initialize (), "second replay initialize");

        for (const NightLightInput& input : trace)
        {
            require (first.update (input).error () == second.update (input).error (),
                     "replay status");

            const NightLightSnapshot left  = first.snapshot  ();
            const NightLightSnapshot right = second.snapshot ();

            require (left.mode == right.mode, "replay mode");
            require (left.sampleState == right.sampleState, "replay sample state");
            require (left.diagnostic == right.diagnostic, "replay diagnostic");
            require (left.status.error () == right.status.error (),
                     "replay snapshot status");
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
