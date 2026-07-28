// Mega 2560, USB 5 V only. Inspect the module markings before wiring:
// joystick X/Y -> A0/A1, joystick SW -> D22, encoder A/B -> D24/D25,
// and encoder SW -> D26. D27-D30 each drive an LED through 1 kOhm to
// GND. D31-D36 drive a 16x2 LCD in 4-bit mode. D5-D7 drive a
// common-cathode RGB LED through one 330 Ohm resistor per channel.
// TP-X, TP-Y, TP-A, and TP-B are the four input signal terminals.
#include <Adk.h>

#include <stdio.h>

namespace {

    const adk::JoystickAxisConfig joystickXConfig                                                            (54, 512, 0, 1023, 48);
    const adk::JoystickAxisConfig joystickYConfig                                                            (55, 512, 0, 1023, 48, true);
    const adk::ButtonConfig       joystickSelectConfig                                                       (22);
    const adk::AnalogJoystickConfig joystickConfig                                                           (
        joystickXConfig,
        joystickYConfig,
        joystickSelectConfig);

    const adk::QuadratureEncoderConfig encoderConfig                                                                 (24, 25);
    const adk::ButtonConfig             cancelButtonConfig                                                           (26);

    const adk::CalibrationConsoleConfig consoleConfig (
        0,
        65535,
        256,
        adk::Duration (1000));

    const adk::Hd44780Pins displayPins = {31, 32, 33, 34, 35, 36};

    const adk::Rgb selectingColor                                  (0, 0, 64);
    const adk::Rgb editingColor                                    (64, 24, 0);
    const adk::Rgb committedColor                                  (0, 64, 0);
    const adk::Rgb cancelledColor                                  (48, 0, 48);
    const adk::Rgb faultColor                                      (64, 0, 0);

    adk::Runtime runtime;

    adk::AnalogJoystick   joystick                                               (runtime.resources (), joystickConfig);
    adk::QuadratureEncoder encoder                                               (runtime.resources (), encoderConfig);
    adk::Button             cancelButton                                         (
        runtime.resources (),
        cancelButtonConfig);

    adk::Hd44780Display display                                (runtime.resources (), displayPins);
    adk::RgbLed stateLed                                       (
        runtime.resources (),
        {5, 330},
        {6, 330},
        {7, 330});
    adk::MonoLed previewLeds[4] = {
        {runtime.resources                           (), 27},
        {runtime.resources                           (), 28},
        {runtime.resources                           (), 29},
        {runtime.resources                           (), 30}};

    adk::CalibrationConsole console (consoleConfig);

    adk::AnalogJoystickSnapshot    joystickEvidence;
    adk::QuadratureEncoderSnapshot encoderEvidence;
    bool                           inputValid = false;
    bool                           halted     = false;

    bool acquireInputs                                     ();
    bool acquireIndicators                                 ();
    bool initializeConsole                                 ();
    bool showReady                                         ();
    void observeControls                                   (adk::TimePoint now);
    void decideCalibration                                 (adk::TimePoint now);
    bool presentPreview                                    (adk::TimePoint now);
    bool presentDisplay                                    (const adk::CalibrationConsoleSnapshot& snapshot);
    bool presentState                                      (adk::CalibrationConsoleState state);
    bool presentPreviewNibble                              (
        const adk::CalibrationConsoleSnapshot& snapshot);
    uint16_t selectedCommitted (
        const adk::CalibrationConsoleSnapshot& snapshot);
    uint16_t selectedPreview (
        const adk::CalibrationConsoleSnapshot& snapshot);
    const char* fieldName                          (adk::CalibrationField field);
    const char* stateName                          (adk::CalibrationConsoleState state);
    void stopSafely                                ();

} // namespace

void setup ()
{
    if (!acquireInputs () ||
        !acquireIndicators                           () ||
        !initializeConsole                           () ||
        !showReady                                   ())
    {
        stopSafely ();
    }
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    observeControls                        (now);
    decideCalibration                      (now);

    if (!presentPreview (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireInputs ()
    {
        if (!joystick.initialize ().ok () ||
            !encoder.initialize                                          ().ok () ||
            !cancelButton.initialize                                     ().ok ())
        {
            return false;
        }

        return true;
    }

    bool acquireIndicators ()
    {
        if (!display.initialize ().ok () ||
            !stateLed.initialize ().ok ())
        {
            return false;
        }

        for (uint8_t bit = 0; bit < 4; ++bit)
        {
            if (!previewLeds[bit].initialize ().ok () ||
                !previewLeds[bit].off ().ok ())
            {
                return false;
            }
        }

        return true;
    }

    bool initializeConsole ()
    {
        return console.initialize (12000, 52000).ok ();
    }

    bool showReady ()
    {
        return display.show (0, "CAL CONSOLE").ok () &&
               display.show                            (1, "SELECT A FIELD").ok () &&
               stateLed.set                            (selectingColor).ok ();
    }

    void observeControls (adk::TimePoint now)
    {
        const adk::Status joystickStatus = joystick.update                                                           (now);
        const adk::Status encoderStatus  = encoder.update                                                            ();

        cancelButton.update (now);

        joystickEvidence = joystick.snapshot                                             ();
        encoderEvidence  = encoder.snapshot                                              ();
        inputValid       = joystickStatus.ok                                             () && encoderStatus.ok () &&
                           cancelButton.initialized ();
    }

    void decideCalibration (adk::TimePoint now)
    {
        adk::CalibrationConsoleInput input;

        input.joystickX   = joystickEvidence.x.position;
        input.joystickY   = joystickEvidence.y.position;
        input.selectEvent = joystickEvidence.selectEvent;
        input.encoderDelta = encoderEvidence.delta;
        input.cancelEvent = cancelButton.pressEvent ();
        input.inputValid  = inputValid;

        console.update (now, input);
    }

    bool presentPreview (adk::TimePoint now)
    {
        const adk::CalibrationConsoleSnapshot snapshot = console.snapshot ();

        return presentDisplay (snapshot) &&
               presentState                                            (snapshot.state) &&
               presentPreviewNibble                                    (snapshot) &&
               display.update                                          (now).ok ();
    }

    bool presentDisplay (const adk::CalibrationConsoleSnapshot& snapshot)
    {
        char committedLine[17];
        char previewLine[17];

        const int committedLength = snprintf (
            committedLine,
            sizeof (committedLine),
            "%s C:%05u",
            fieldName                                      (snapshot.field),
            selectedCommitted                              (snapshot));
        const int previewLength = snprintf (
            previewLine,
            sizeof (previewLine),
            "P:%05u %-6s",
            selectedPreview                            (snapshot),
            stateName                                  (snapshot.state));

        return committedLength >= 0 && committedLength <= 16 &&
               previewLength >= 0 && previewLength <= 16 &&
               display.show                            (0, committedLine).ok () &&
               display.show                            (1, previewLine).ok ();
    }

    bool presentState (adk::CalibrationConsoleState state)
    {
        switch (state)
        {
            case adk::CalibrationConsoleState::Selecting:
                return stateLed.set (selectingColor).ok ();
            case adk::CalibrationConsoleState::Editing:
                return stateLed.set (editingColor).ok ();
            case adk::CalibrationConsoleState::Committed:
                return stateLed.set (committedColor).ok ();
            case adk::CalibrationConsoleState::Cancelled:
                return stateLed.set (cancelledColor).ok ();
            case adk::CalibrationConsoleState::Fault:
                return stateLed.set (faultColor).ok ();
        }

        return false;
    }

    bool presentPreviewNibble (
        const adk::CalibrationConsoleSnapshot& snapshot)
    {
        const uint8_t highNibble =
            static_cast<uint8_t> (selectedPreview (snapshot) >> 12);

        for (uint8_t bit = 0; bit < 4; ++bit)
        {
            if (!previewLeds[bit].set ((highNibble & (1U << bit)) != 0).ok ())
            {
                return false;
            }
        }

        return true;
    }

    uint16_t selectedCommitted (
        const adk::CalibrationConsoleSnapshot& snapshot)
    {
        return snapshot.field == adk::CalibrationField::Minimum
                   ? snapshot.committedMinimum
                   : snapshot.committedMaximum;
    }

    uint16_t selectedPreview (
        const adk::CalibrationConsoleSnapshot& snapshot)
    {
        return snapshot.field == adk::CalibrationField::Minimum
                   ? snapshot.previewMinimum
                   : snapshot.previewMaximum;
    }

    const char* fieldName (adk::CalibrationField field)
    {
        return field == adk::CalibrationField::Minimum ? "MIN" : "MAX";
    }

    const char* stateName (adk::CalibrationConsoleState state)
    {
        switch (state)
        {
            case adk::CalibrationConsoleState::Selecting: return "SELECT";
            case adk::CalibrationConsoleState::Editing:   return "EDIT";
            case adk::CalibrationConsoleState::Committed: return "SAVED";
            case adk::CalibrationConsoleState::Cancelled: return "CANCEL";
            case adk::CalibrationConsoleState::Fault:     return "FAULT";
        }

        return "FAULT";
    }

    void stopSafely ()
    {
        halted = true;

        console.shutdown                                   ();
        cancelButton.shutdown                              ();
        encoder.shutdown                                   ();
        joystick.shutdown                                  ();
        display.shutdown                                   ();

        for (uint8_t bit = 0; bit < 4; ++bit)
        {
            previewLeds[bit].shutdown ();
        }

        stateLed.shutdown ();
    }

} // namespace
