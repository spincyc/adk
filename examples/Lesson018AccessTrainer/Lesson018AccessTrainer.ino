#include <Adk.h>
#include <access_trainer.h>

// Mega 2560, USB 5 V only: 4x3 keypad D22-D28, common-cathode RGB
// on D5-D7, soft-latch intent LED on D30, LCD on D34-D39, ready on D13.
// This inert trainer has no servo, motor, relay, lock, or external load supply.

namespace {

    const adk::MatrixKeypadPins keypadPins = {22, 23, 24, 25, 26, 27, 28};

    const adk::Hd44780Pins displayPins = {34, 35, 36, 37, 38, 39};

    adk::AccessTrainerConfig makeTrainerConfig ();

    const adk::AccessTrainerConfig trainerConfig = makeTrainerConfig ();

    adk::Runtime        runtime;
    adk::MatrixKeypad   keypad          (runtime.resources (), keypadPins);
    adk::RgbLed         stateLed        (runtime.resources (),
                                         {5, 220},
                                         {6, 220},
                                         {7, 220});
    adk::MonoLed        softLatchIntent (runtime.resources (), 30);
    adk::Hd44780Display display         (runtime.resources (), displayPins);
    adk::MonoLed        acquisitionLed  (runtime.resources (), LED_BUILTIN);
    adk::AccessTrainer  trainer         (trainerConfig);

    bool running           = false;
    bool presentationFault = false;
    bool auditVisible      = false;

    adk::TimePoint       auditShownAt     (0);
    adk::AccessAuditKind shownAuditKind     = adk::AccessAuditKind::Reset;
    uint16_t             shownAuditSequence = 0;

    bool                acquireTrainerCircuit ();
    adk::AccessInput    observeOperator       (adk::TimePoint now);
    adk::AccessSnapshot decideAccess          (adk::TimePoint          now,
                                               const adk::AccessInput& input);
    bool                presentAccess         (adk::TimePoint          now,
                                               const adk::AccessSnapshot& decision);
    adk::Rgb            stateColor            (adk::TimePoint      now,
                                               adk::AccessLedIntent intent);
    void                consumeAuditRecord    (adk::TimePoint          now,
                                               const adk::AccessSnapshot& decision);
    bool                showPrompt            (adk::TimePoint          now,
                                               const adk::AccessSnapshot& decision);
    bool                showAuditRecord       ();
    const char*         auditPrompt           (adk::AccessAuditKind kind);
    const char*         statePrompt           (adk::AccessState state);
    void                stopSafely            ();

} // namespace

void setup ()
{
    running = acquireTrainerCircuit ();

    if (!running)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (static_cast<uint32_t> (millis ()));

    const adk::AccessInput    input    = observeOperator                    (now);
    const adk::AccessSnapshot decision = decideAccess                       (now, input);

    if (!presentAccess (now, decision))
    {
        stopSafely ();
    }
}

namespace {

    adk::AccessTrainerConfig makeTrainerConfig ()
    {
        adk::AccessTrainerConfig config;

        config.credential[0]         = adk::KeypadKey::Digit1;
        config.credential[1]         = adk::KeypadKey::Digit2;
        config.credential[2]         = adk::KeypadKey::Digit3;
        config.credential[3]         = adk::KeypadKey::Digit4;
        config.credentialLength      = 4;
        config.maximumFailedAttempts = 3;
        config.grantDuration         = adk::Duration (3000);
        config.deniedDuration        = adk::Duration (1000);
        config.lockoutDuration       = adk::Duration (10000);
        return config;
    }

    bool acquireTrainerCircuit ()
    {
        if (!keypad.initialize ().ok ())
        {
            return false;
        }

        if (!stateLed.initialize ().ok ())
        {
            keypad.shutdown ();
            return false;
        }

        if (!softLatchIntent.initialize ().ok ())
        {
            stateLed.shutdown                                               ();
            keypad.shutdown                                                 ();
            return false;
        }

        if (!display.initialize ().ok ())
        {
            softLatchIntent.shutdown                                        ();
            stateLed.shutdown                                               ();
            keypad.shutdown                                                 ();
            return false;
        }

        if (!acquisitionLed.initialize ().ok ())
        {
            display.shutdown                                                ();
            softLatchIntent.shutdown                                        ();
            stateLed.shutdown                                               ();
            keypad.shutdown                                                 ();
            return false;
        }

        if (!trainer.initialize ().ok ())
        {
            acquisitionLed.shutdown                                         ();
            display.shutdown                                                ();
            softLatchIntent.shutdown                                        ();
            stateLed.shutdown                                               ();
            keypad.shutdown                                                 ();
            return false;
        }

        return acquisitionLed.on ().ok ();
    }

    adk::AccessInput observeOperator (adk::TimePoint now)
    {
        const adk::Status keypadStatus  = keypad.update                      (now);
        const adk::Status displayStatus = display.update                     (now);
        const bool        componentFault =
            presentationFault || !keypadStatus.ok () || !displayStatus.ok ();

        return adk::AccessInput (keypad.snapshot (), componentFault);
    }

    adk::AccessSnapshot decideAccess (adk::TimePoint now, const adk::AccessInput& input)
    {
        trainer.update          (now, input);
        return trainer.snapshot ();
    }

    bool presentAccess (adk::TimePoint now, const adk::AccessSnapshot& decision)
    {
        consumeAuditRecord (now, decision);

        if (!stateLed.set (stateColor (now, decision.ledIntent)).ok ())
        {
            return false;
        }

        const adk::Status latchStatus =
            decision.softLatchOpen ? softLatchIntent.on () : softLatchIntent.off ();
        if (!latchStatus.ok ())
        {
            return false;
        }

        presentationFault = !showPrompt (now, decision);
        return true;
    }

    adk::Rgb stateColor (adk::TimePoint now, adk::AccessLedIntent intent)
    {
        switch (intent)
        {
            case adk::AccessLedIntent::Ready:     return adk::Rgb (0,  0,  96);
            case adk::AccessLedIntent::Entering:  return adk::Rgb (64, 64, 64);
            case adk::AccessLedIntent::Granted:   return adk::Rgb (0,  96, 0);
            case adk::AccessLedIntent::Denied:    return adk::Rgb (96, 32, 0);
            case adk::AccessLedIntent::LockedOut:
                return (now.milliseconds () % 1000U) < 500U ? adk::Rgb (96, 32, 0)
                                                            : adk::Rgb (0, 0, 0);
            case adk::AccessLedIntent::Fault:     return adk::Rgb (96, 0, 0);
        }

        return adk::Rgb (96, 0, 0);
    }

    void consumeAuditRecord (adk::TimePoint now, const adk::AccessSnapshot& decision)
    {
        if (!decision.hasAuditRecord)
        {
            return;
        }

        shownAuditKind     = decision.auditRecord.kind;
        shownAuditSequence = decision.auditRecord.sequence;
        auditShownAt       = now;
        auditVisible       = true;
    }

    bool showPrompt (adk::TimePoint now, const adk::AccessSnapshot& decision)
    {
        char countLine[] = "ENTRY COUNT: 0  ";

        countLine[13] = static_cast<char> ('0' + decision.enteredCount);

        if (auditVisible && now.elapsedSince (auditShownAt).milliseconds () < 1000U)
        {
            return display.show (0, statePrompt (decision.state)).ok () &&
                   showAuditRecord ();
        }

        auditVisible = false;
        return display.show (0, statePrompt (decision.state)).ok () &&
               display.show (1, countLine).ok ();
    }

    bool showAuditRecord ()
    {
        char     auditLine[] = "A0000 RESET     ";
        uint16_t sequence    = shownAuditSequence;

        for (uint8_t index = 5; index > 1; --index)
        {
            auditLine[index - 1] = static_cast<char> ('0' + sequence % 10U);
            sequence             = static_cast<uint16_t> (sequence / 10U);
        }

        const char* prompt = auditPrompt (shownAuditKind);
        for (uint8_t index = 0; index < 10; ++index)
        {
            auditLine[index + 6] = prompt[index];
        }

        return display.show (1, auditLine).ok ();
    }

    const char* auditPrompt (adk::AccessAuditKind kind)
    {
        switch (kind)
        {
            case adk::AccessAuditKind::Granted:        return "GRANTED   ";
            case adk::AccessAuditKind::Denied:         return "DENIED    ";
            case adk::AccessAuditKind::LockoutStarted: return "LOCKOUT   ";
            case adk::AccessAuditKind::GrantExpired:   return "GRANT END ";
            case adk::AccessAuditKind::DeniedExpired:  return "DENY END  ";
            case adk::AccessAuditKind::LockoutExpired: return "LOCK END  ";
            case adk::AccessAuditKind::Fault:          return "FAULT     ";
            case adk::AccessAuditKind::Reset:          return "RESET     ";
        }

        return "FAULT     ";
    }

    const char* statePrompt (adk::AccessState state)
    {
        switch (state)
        {
            case adk::AccessState::Ready:     return "READY           ";
            case adk::AccessState::Entering:  return "ENTER           ";
            case adk::AccessState::Granted:   return "GRANTED         ";
            case adk::AccessState::Denied:    return "DENIED          ";
            case adk::AccessState::LockedOut: return "LOCKED OUT      ";
            case adk::AccessState::Fault:     return "FAULT           ";
        }

        return "FAULT           ";
    }

    void stopSafely ()
    {
        trainer.shutdown                                                     ();
        softLatchIntent.off                                                  ();
        stateLed.off                                                         ();
        acquisitionLed.off                                                   ();
        acquisitionLed.shutdown                                              ();
        display.shutdown                                                     ();
        softLatchIntent.shutdown                                             ();
        stateLed.shutdown                                                    ();
        keypad.shutdown                                                      ();
        running = false;
    }

} // namespace
