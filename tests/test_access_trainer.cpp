#include "access_trainer.h"

#include <stdio.h>
#include <stdlib.h>

namespace {

    void require (bool condition)
    {
        if (!condition)
        {
            abort ();

        }
    }

    void requireStatus (adk::Status status, adk::StatusCode expected)
    {
        require (status.error () == expected);
    }

    adk::AccessTrainerConfig config ()
    {
        adk::AccessTrainerConfig result;

        result.credential[0]         = adk::KeypadKey::Digit1;
        result.credential[1]         = adk::KeypadKey::Digit2;
        result.credential[2]         = adk::KeypadKey::Digit3;
        result.credential[3]         = adk::KeypadKey::Digit4;
        result.credentialLength      = 4;
        result.maximumFailedAttempts = 3;
        result.grantDuration         = adk::Duration (100);

        result.deniedDuration        = adk::Duration (20);

        result.lockoutDuration       = adk::Duration (200);

        return result;
    }

    adk::AccessInput idleInput ()
    {
        const adk::KeypadSnapshot snapshot = {
            adk::KeypadKey::None,
            adk::KeypadState::Released,
            adk::StatusCode::Ok,
            0,
            false,
            false
        };

        return adk::AccessInput (snapshot);

    }

    adk::AccessInput press (adk::KeypadKey key)
    {
        adk::AccessInput result = idleInput ();


        result.keypad.key        = key;
        result.keypad.state      = adk::KeypadState::Pressed;
        result.keypad.pressEvent = true;
        return result;
    }

    void enter (adk::AccessTrainer& trainer,
                uint32_t            start,
                const adk::KeypadKey* keys,
                uint8_t             count)
    {
        for (uint8_t index = 0; index < count; ++index)
        {
            requireStatus (trainer.update (adk::TimePoint (start + index),
                                    press (keys[index])), adk::StatusCode::Ok);

        }
    }

    void submitWrong (adk::AccessTrainer& trainer, uint32_t now)
    {
        requireStatus (trainer.update (adk::TimePoint (now),
                                press (adk::KeypadKey::Digit9)), adk::StatusCode::Ok);

        requireStatus (trainer.update (adk::TimePoint (now + 1),
                                press (adk::KeypadKey::Hash)), adk::StatusCode::Ok);

    }

    void testInvalidConfiguration ()
    {
        adk::AccessTrainerConfig bad;
        adk::AccessTrainer       trainer (bad);


        requireStatus (trainer.initialize (), adk::StatusCode::InvalidArgument);

        require (trainer.snapshot ().state == adk::AccessState::Fault);

        require (!trainer.snapshot ().softLatchOpen);


        bad = config ();

        bad.credential[2] = adk::KeypadKey::Hash;
        adk::AccessTrainer badCredential (bad);

        requireStatus (badCredential.initialize (), adk::StatusCode::InvalidArgument);

    }

    void testGrantAndClose ()
    {
        const adk::KeypadKey credential[] = {
            adk::KeypadKey::Digit1,
            adk::KeypadKey::Digit2,
            adk::KeypadKey::Digit3,
            adk::KeypadKey::Digit4
        };
        adk::AccessTrainer trainer (config ());


        requireStatus (trainer.initialize (), adk::StatusCode::Ok);

        requireStatus (trainer.initialize (), adk::StatusCode::Ok);

        enter (trainer, 10, credential, 4);

        require (trainer.snapshot ().enteredCount == 4);

        requireStatus (trainer.update (adk::TimePoint (14),
                                press (adk::KeypadKey::Hash)), adk::StatusCode::Ok);


        adk::AccessSnapshot view = trainer.snapshot ();

        require (view.state == adk::AccessState::Granted);

        require (view.softLatchOpen);

        require (view.softLatchIntent == adk::SoftLatchIntent::Open);

        require (view.enteredCount == 0);

        require (view.hasAuditRecord);


        requireStatus (trainer.update (adk::TimePoint (113), idleInput ()), adk::StatusCode::Ok);

        require (trainer.snapshot ().state == adk::AccessState::Granted);

        requireStatus (trainer.update (adk::TimePoint (114), idleInput ()), adk::StatusCode::Ok);

        view = trainer.snapshot ();

        require (view.state == adk::AccessState::Ready);

        require (!view.softLatchOpen);

        require (view.softLatchIntent == adk::SoftLatchIntent::Closed);

        require (view.clearEntry);

        require (trainer.update (adk::TimePoint (115), idleInput ()).ok ());
        require (trainer.snapshot ().state == adk::AccessState::Ready);

    }

    void testClearChordAndLockout ()
    {
        adk::AccessTrainer trainer (config ());

        requireStatus (trainer.initialize (), adk::StatusCode::Ok);


        requireStatus (trainer.update (adk::TimePoint (0),
                                press (adk::KeypadKey::Digit1)), adk::StatusCode::Ok);

        requireStatus (trainer.update (adk::TimePoint (1),
                                press (adk::KeypadKey::Star)), adk::StatusCode::Ok);

        require (trainer.snapshot ().state == adk::AccessState::Ready);

        require (trainer.snapshot ().enteredCount == 0);

        adk::AccessInput chord = idleInput ();

        chord.keypad.state      = adk::KeypadState::InvalidChord;
        chord.keypad.pressEvent = true;
        chord.keypad.rawMask    = 3;
        requireStatus (trainer.update (adk::TimePoint (2), chord), adk::StatusCode::Ok);

        require (trainer.snapshot ().enteredCount == 0);


        submitWrong (trainer, 10);

        require (trainer.snapshot ().state == adk::AccessState::Denied);

        requireStatus (trainer.update (adk::TimePoint (31), idleInput ()), adk::StatusCode::Ok);


        submitWrong (trainer, 40);

        requireStatus (trainer.update (adk::TimePoint (61), idleInput ()), adk::StatusCode::Ok);


        submitWrong (trainer, 70);

        require (trainer.snapshot ().state == adk::AccessState::LockedOut);

        require (trainer.snapshot ().hasAuditRecord);

        require (trainer.snapshot ().auditRecord.kind ==
                 adk::AccessAuditKind::LockoutStarted);

        require (trainer.snapshot ().failedAttempts == 3);

        require (!trainer.snapshot ().softLatchOpen);


        requireStatus (trainer.update (adk::TimePoint (270),
                                press (adk::KeypadKey::Digit1)), adk::StatusCode::Ok);

        require (trainer.snapshot ().state == adk::AccessState::LockedOut);

        requireStatus (trainer.update (adk::TimePoint (271), idleInput ()), adk::StatusCode::Ok);

        require (trainer.snapshot ().state == adk::AccessState::Ready);

        require (trainer.update (adk::TimePoint (272), idleInput ()).ok ());
        require (trainer.snapshot ().state == adk::AccessState::Ready);

        require (trainer.snapshot ().failedAttempts == 0);

        require (trainer.snapshot ().enteredCount == 0);

    }

    void testFaultResetAuditAndWrap ()
    {
        adk::AccessTrainer trainer (config ());

        requireStatus (trainer.initialize (), adk::StatusCode::Ok);

        requireStatus (trainer.reset (adk::TimePoint (0xfffffff0u)), adk::StatusCode::Ok);


        adk::AccessInput fault = idleInput ();

        fault.componentFault   = true;
        requireStatus (trainer.update (adk::TimePoint (0xfffffff1u), fault), adk::StatusCode::HardwareFailure);

        require (trainer.snapshot ().state == adk::AccessState::Fault);

        require (!trainer.snapshot ().softLatchOpen);

        require (trainer.snapshot ().hasAuditRecord);


        requireStatus (trainer.update (adk::TimePoint (0xfffffff2u), idleInput ()), adk::StatusCode::HardwareFailure);

        require (!trainer.snapshot ().hasAuditRecord);

        requireStatus (trainer.reset (adk::TimePoint (0xfffffff3u)), adk::StatusCode::Ok);


        submitWrong (trainer, 0xfffffff4u);

        requireStatus (trainer.update (adk::TimePoint (8), idleInput ()), adk::StatusCode::Ok);

        require (trainer.snapshot ().state == adk::AccessState::Denied);

        requireStatus (trainer.update (adk::TimePoint (9), idleInput ()), adk::StatusCode::Ok);

        require (trainer.snapshot ().state == adk::AccessState::Ready);


        requireStatus (trainer.update (adk::TimePoint (10), idleInput ()), adk::StatusCode::Ok);

        require (!trainer.snapshot ().hasAuditRecord);


        requireStatus (trainer.update (adk::TimePoint (0xfffffff0u), idleInput ()), adk::StatusCode::InvalidArgument);

        require (trainer.snapshot ().state == adk::AccessState::Fault);

        trainer.shutdown ();

        require (trainer.snapshot ().status.error () == adk::StatusCode::NotInitialized);

        require (!trainer.snapshot ().softLatchOpen);

    }

    void testBoundedEntry ()
    {
        adk::AccessTrainer trainer (config ());

        require (trainer.initialize ().ok ());

        for (uint8_t index = 0;
             index < adk::AccessTrainerConfig::credentialCapacity + 1;
             ++index)
        {
            require (trainer.update (adk::TimePoint (index),
                                     press (adk::KeypadKey::Digit1)).ok ());
        }

        require (
            trainer.snapshot ().enteredCount ==
            adk::AccessTrainerConfig::credentialCapacity);

        require (trainer.update (
            adk::TimePoint (20), press (adk::KeypadKey::Hash)).ok ());

        require (trainer.snapshot ().state == adk::AccessState::Denied);

        require (trainer.snapshot ().auditRecord.kind ==
                 adk::AccessAuditKind::Denied);
    }

    void testEquivalentDenialsAndStuckKey ()
    {
        adk::AccessTrainer shortEntry (config ());
        adk::AccessTrainer wrongValue (config ());

        require (shortEntry.initialize ().ok ());
        require (wrongValue.initialize ().ok ());

        require (shortEntry.update (
            adk::TimePoint (1), press (adk::KeypadKey::Digit1)).ok ());

        require (wrongValue.update (
            adk::TimePoint (1), press (adk::KeypadKey::Digit9)).ok ());

        require (shortEntry.update (
            adk::TimePoint (2), press (adk::KeypadKey::Hash)).ok ());

        require (wrongValue.update (
            adk::TimePoint (2), press (adk::KeypadKey::Hash)).ok ());

        const adk::AccessSnapshot shortView = shortEntry.snapshot ();
        const adk::AccessSnapshot wrongView = wrongValue.snapshot ();

        require (shortView.state == wrongView.state);
        require (shortView.failedAttempts == wrongView.failedAttempts);
        require (shortView.auditRecord.kind == wrongView.auditRecord.kind);
        require (shortView.auditRecord.time == wrongView.auditRecord.time);

        require (shortEntry.reset (adk::TimePoint (3)).ok ());

        adk::AccessInput held = press (adk::KeypadKey::Digit1);

        held.keypad.pressEvent = false;

        require (shortEntry.update (adk::TimePoint (4), held).ok ());
        require (shortEntry.snapshot ().enteredCount == 0);
    }

    void testFaultAndLifecycleCoverage ()
    {
        adk::AccessTrainer trainer (config ());

        require (trainer.initialize ().ok ());
        require (trainer.reset (adk::TimePoint (0)).ok ());
        require (trainer.snapshot ().state == adk::AccessState::Ready);

        require (trainer.update (
            adk::TimePoint (1), press (adk::KeypadKey::Digit1)).ok ());

        require (trainer.snapshot ().state == adk::AccessState::Entering);

        adk::AccessInput keypadFault = idleInput ();

        keypadFault.keypad.state  = adk::KeypadState::Fault;
        keypadFault.keypad.status = adk::StatusCode::HardwareFailure;

        requireStatus (
            trainer.update (adk::TimePoint (2), keypadFault),
            adk::StatusCode::HardwareFailure);

        require (trainer.snapshot ().state == adk::AccessState::Fault);
        require (trainer.snapshot ().softLatchIntent ==
                 adk::SoftLatchIntent::Closed);
        require (trainer.reset (adk::TimePoint (3)).ok ());

        const adk::KeypadKey credential[] = {
            adk::KeypadKey::Digit1,
            adk::KeypadKey::Digit2,
            adk::KeypadKey::Digit3,
            adk::KeypadKey::Digit4
        };

        enter (trainer, 4, credential, 4);

        require (trainer.update (
            adk::TimePoint (8), press (adk::KeypadKey::Hash)).ok ());

        require (trainer.snapshot ().state == adk::AccessState::Granted);

        adk::AccessInput componentFault = idleInput ();

        componentFault.componentFault = true;

        requireStatus (
            trainer.update (adk::TimePoint (9), componentFault),
            adk::StatusCode::HardwareFailure);

        require (trainer.snapshot ().state == adk::AccessState::Fault);
        require (!trainer.snapshot ().softLatchOpen);
        require (trainer.reset (adk::TimePoint (10)).ok ());

        submitWrong (trainer, 11);

        require (trainer.snapshot ().state == adk::AccessState::Denied);

        require (trainer.reset (adk::TimePoint (13)).ok ());

        submitWrong (trainer, 14);

        require (trainer.update (adk::TimePoint (35), idleInput ()).ok ());

        submitWrong (trainer, 36);

        require (trainer.update (adk::TimePoint (57), idleInput ()).ok ());

        submitWrong (trainer, 58);

        require (trainer.snapshot ().state == adk::AccessState::LockedOut);

        require (trainer.reset (adk::TimePoint (60)).ok ());

        trainer.shutdown ();

        require (trainer.snapshot ().state == adk::AccessState::Fault);
        require (trainer.snapshot ().softLatchIntent ==
                 adk::SoftLatchIntent::Closed);
        require (trainer.update (adk::TimePoint (61), idleInput ()).error () ==
                 adk::StatusCode::NotInitialized);
    }

    void testReplayAndAuditRollover ()
    {
        adk::AccessTrainer left  (config ());
        adk::AccessTrainer right (config ());

        require (left.initialize ().ok ());
        require (right.initialize ().ok ());

        const adk::KeypadKey trace[] = {
            adk::KeypadKey::Digit1,
            adk::KeypadKey::Digit2,
            adk::KeypadKey::Digit3,
            adk::KeypadKey::Digit4,
            adk::KeypadKey::Hash
        };

        for (uint8_t index = 0; index < 5; ++index)
        {
            require (left.update (
                adk::TimePoint (index), press (trace[index])).ok ());

            require (right.update (
                adk::TimePoint (index), press (trace[index])).ok ());

            const adk::AccessSnapshot leftView  = left.snapshot  ();
            const adk::AccessSnapshot rightView = right.snapshot ();

            require (leftView.state == rightView.state);
            require (leftView.status == rightView.status);
            require (leftView.softLatchIntent == rightView.softLatchIntent);
            require (leftView.enteredCount == rightView.enteredCount);
            require (leftView.failedAttempts == rightView.failedAttempts);
            require (leftView.hasAuditRecord == rightView.hasAuditRecord);

            if (leftView.hasAuditRecord)
            {
                require (
                    leftView.auditRecord.sequence ==
                    rightView.auditRecord.sequence);

                require (
                    leftView.auditRecord.kind == rightView.auditRecord.kind);
            }
        }

        adk::AccessTrainer rollover (config ());

        require (rollover.initialize ().ok ());

        for (uint32_t index = 0; index <= UINT16_MAX; ++index)
        {
            require (rollover.reset (adk::TimePoint (index)).ok ());
        }

        require (rollover.snapshot ().auditRecord.sequence == UINT16_MAX);
        require (rollover.reset (adk::TimePoint (UINT16_MAX + 1u)).ok ());
        require (rollover.snapshot ().auditRecord.sequence == 0);
    }
}

int main ()
{
    testInvalidConfiguration ();

    testGrantAndClose       ();

    testClearChordAndLockout ();

    testFaultResetAuditAndWrap ();

    testBoundedEntry ();

    testEquivalentDenialsAndStuckKey ();

    testFaultAndLifecycleCoverage ();

    testReplayAndAuditRollover ();

    puts ("All ADK access trainer tests passed.");

}
