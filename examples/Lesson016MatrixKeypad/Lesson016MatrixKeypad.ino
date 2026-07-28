#include <Adk.h>

// Mega 2560, USB 5 V: use the 1-2-3 through *-0-# section of the authorized
// passive 4x4 keypad. Its four rows use D22-D25 and its first three columns
// use D26-D28. Identify all eight tail conductors first; insulate the unused
// A-B-C-D column conductor separately. D13 celebrates accepted input.

namespace {

    const adk::MatrixKeypadPins keypadPins =
    {
        22, 23, 24, 25,
        26, 27, 28
    };
    const adk::KeypadConfig keypadConfig (adk::Duration (20));

    adk::Runtime      runtime;
    adk::MatrixKeypad keypad   (runtime.resources (), keypadPins, keypadConfig);
    adk::MonoLed      evidence (runtime.resources (), LED_BUILTIN);

    enum struct EvidenceKind : uint8_t
    {
        None,
        AcceptedKey,
        InvalidChord,
        ScanFault
    };

    struct OperatorDecision
    {
        EvidenceKind kind;
        uint8_t      pulseCount;
        uint16_t     onTimeMs;
        uint16_t     offTimeMs;
        bool         repeating;
    };

    struct EvidencePattern
    {
        uint32_t       deadlineMs;
        uint8_t        pulseCount;
        uint8_t        completedPulses;
        uint16_t       onTimeMs;
        uint16_t       offTimeMs;
        bool           active;
        bool           lit;
        bool           repeating;
    };

    EvidencePattern pattern = {};
    bool            running = false;

    bool                  initializeOperatorPanel ();
    adk::KeypadSnapshot   observeKeypad           (adk::TimePoint now);
    OperatorDecision      decideOperatorEvent     (const adk::KeypadSnapshot& observation);
    bool                  showOperatorEvidence    (adk::TimePoint now,
                                                   const OperatorDecision& decision);
    bool                  advanceEvidence         (adk::TimePoint now);
    OperatorDecision      acceptedKeyPattern      (adk::KeypadKey key);
    bool                  startPattern            (adk::TimePoint now,
                                                   const OperatorDecision& decision);
    void                  stopSafely              ();
} // namespace

void setup ()
{
    running = initializeOperatorPanel ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint      now                               (millis ());
    const adk::KeypadSnapshot observation = observeKeypad       (now);
    const OperatorDecision    decision    = decideOperatorEvent (observation);

    if (!showOperatorEvidence (now, decision))
    {
        stopSafely ();
    }
}

namespace {

    bool initializeOperatorPanel ()
    {
        if (!keypad.initialize ().ok ())
        {
            return false;
        }

        if (!evidence.initialize ().ok ())
        {
            keypad.shutdown ();
            return false;
        }

        return startPattern (adk::TimePoint (millis ()),
                             {EvidenceKind::AcceptedKey, 1, 100, 100, false});
    }

    adk::KeypadSnapshot observeKeypad (adk::TimePoint now)
    {
        keypad.update          (now);
        return keypad.snapshot ();
    }

    OperatorDecision decideOperatorEvent (const adk::KeypadSnapshot& observation)
    {
        if (!observation.status.ok ()                     ||
            observation.state == adk::KeypadState::Fault)
        {
            return {EvidenceKind::ScanFault, 3, 100, 100, true};
        }

        if (observation.state == adk::KeypadState::InvalidChord)
        {
            return {EvidenceKind::InvalidChord, 2, 100, 100, true};
        }

        if (observation.pressEvent)
        {
            return acceptedKeyPattern (observation.key);
        }

        return {EvidenceKind::None, 0, 0, 0, false};
    }

    bool showOperatorEvidence (adk::TimePoint now, const OperatorDecision& decision)
    {
        if (decision.kind == EvidenceKind::None &&
            pattern.active                          &&
            pattern.repeating)
        {
            if (!evidence.off ().ok ())
            {
                return false;
            }

            pattern.active = false;
            pattern.lit    = false;
        }

        if (decision.kind == EvidenceKind::ScanFault ||
            decision.kind == EvidenceKind::InvalidChord)
        {
            if (!pattern.active ||
                pattern.pulseCount != decision.pulseCount ||
                !pattern.repeating)
            {
                if (!startPattern (now, decision))
                {
                    return false;
                }
            }
        }
        else if (decision.kind == EvidenceKind::AcceptedKey && !pattern.active)
        {
            if (!startPattern (now, decision))
            {
                return false;
            }
        }

        return advanceEvidence (now);
    }

    bool advanceEvidence (adk::TimePoint now)
    {
        const uint32_t nowMs = now.milliseconds ();

        if (!pattern.active ||
            static_cast<int32_t> (nowMs - pattern.deadlineMs) < 0)
        {
            return true;
        }

        if (pattern.lit)
        {
            if (!evidence.off ().ok ())
            {
                return false;
            }

            pattern.lit        = false;
            pattern.deadlineMs = nowMs + pattern.offTimeMs;
            return true;
        }

        ++pattern.completedPulses;

        if (pattern.completedPulses >= pattern.pulseCount)
        {
            if (!pattern.repeating)
            {
                pattern.active = false;
                return true;
            }

            pattern.completedPulses = 0;
            pattern.deadlineMs      = nowMs + 400U;
            return true;
        }

        if (!evidence.on ().ok ())
        {
            return false;
        }

        pattern.lit        = true;
        pattern.deadlineMs = nowMs + pattern.onTimeMs;
        return true;
    }

    OperatorDecision acceptedKeyPattern (adk::KeypadKey key)
    {
        switch (key)
        {
            case adk::KeypadKey::Digit0: return {EvidenceKind::AcceptedKey, 10, 100, 100, false};
            case adk::KeypadKey::Digit1: return {EvidenceKind::AcceptedKey, 1,  100, 100, false};
            case adk::KeypadKey::Digit2: return {EvidenceKind::AcceptedKey, 2,  100, 100, false};
            case adk::KeypadKey::Digit3: return {EvidenceKind::AcceptedKey, 3,  100, 100, false};
            case adk::KeypadKey::Digit4: return {EvidenceKind::AcceptedKey, 4,  100, 100, false};
            case adk::KeypadKey::Digit5: return {EvidenceKind::AcceptedKey, 5,  100, 100, false};
            case adk::KeypadKey::Digit6: return {EvidenceKind::AcceptedKey, 6,  100, 100, false};
            case adk::KeypadKey::Digit7: return {EvidenceKind::AcceptedKey, 7,  100, 100, false};
            case adk::KeypadKey::Digit8: return {EvidenceKind::AcceptedKey, 8,  100, 100, false};
            case adk::KeypadKey::Digit9: return {EvidenceKind::AcceptedKey, 9,  100, 100, false};
            case adk::KeypadKey::Star:   return {EvidenceKind::AcceptedKey, 1,  500, 250, false};
            case adk::KeypadKey::Hash:   return {EvidenceKind::AcceptedKey, 2,  500, 250, false};
            case adk::KeypadKey::None:   return {EvidenceKind::None,        0,  0,   0,   false};
        }

        return {EvidenceKind::ScanFault, 3, 100, 100, true};
    }

    bool startPattern (adk::TimePoint now, const OperatorDecision& decision)
    {
        if (!evidence.off ().ok ())
        {
            return false;
        }

        pattern.deadlineMs      = now.milliseconds () + decision.onTimeMs;
        pattern.pulseCount      = decision.pulseCount;
        pattern.completedPulses = 0;
        pattern.onTimeMs        = decision.onTimeMs;
        pattern.offTimeMs       = decision.offTimeMs;
        pattern.active          = decision.pulseCount != 0;
        pattern.lit             = decision.pulseCount != 0;
        pattern.repeating       = decision.repeating;

        return !pattern.lit || evidence.on ().ok ();
    }

    void stopSafely ()
    {
        evidence.shutdown ();
        keypad.shutdown   ();
        running = false;
    }
} // namespace
