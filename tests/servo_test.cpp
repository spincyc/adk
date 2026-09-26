#include "check.h"

#include <adk/servo.h>
#include <Arduino.h>

namespace {

    const uint8_t Mode14    = _BV (WGM51);
    const uint8_t Clock     = _BV (WGM53) | _BV (WGM52) | _BV (CS51);
    const uint8_t Connected = _BV (COM5A1) | _BV (COM5B1) | _BV (COM5C1);
}

TEST (servoSetsUpTimer5ButSendsNoPulsesYet)
{
    adk::Servo servo {44};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (44).mode == OUTPUT);
    CHECK (arduino::pin (44).output == LOW);
    CHECK (TCCR5A == Mode14);
    CHECK (TCCR5B == Clock);
    CHECK (ICR5 == 39999);
    CHECK (servo.angle () == 90);
    CHECK (!servo.isMoving ());
}

TEST (servoPulseWidthFollowsTheAngle)
{
    adk::Servo servo {44};

    adk::setup ();

    servo.write (0);
    CHECK (TCCR5A == (Mode14 | _BV (COM5C1)));
    CHECK (OCR5C == 1088);
    CHECK (servo.angle () == 0);

    servo.write (90);
    CHECK (OCR5C == 2944);
    CHECK (servo.angle () == 90);

    servo.write (180);
    CHECK (OCR5C == 4800);
    CHECK (servo.angle () == 180);

    servo.write (1);
    CHECK (OCR5C == 1108);
    CHECK (servo.angle () == 1);

    servo.write (250);
    CHECK (OCR5C == 4800);
    CHECK (servo.angle () == 180);
}

TEST (servoPulseWidthsAreKeptWithinItsRange)
{
    adk::Servo servo {45, 1000, 2000};

    adk::setup ();

    servo.writeMicroseconds (1500);
    CHECK (OCR5B == 3000);
    CHECK (servo.angle () == 90);

    servo.writeMicroseconds (400);
    CHECK (OCR5B == 2000);
    CHECK (servo.angle () == 0);

    servo.writeMicroseconds (2600);
    CHECK (OCR5B == 4000);
    CHECK (servo.angle () == 180);
}

TEST (servosShareTimer5)
{
    adk::Servo a {44};
    adk::Servo b {45};
    adk::Servo c {46};

    adk::setup ();
    CHECK (!check::halted.happened);

    a.write (0);
    b.write (90);
    c.write (180);

    CHECK (OCR5C == 1088);
    CHECK (OCR5B == 2944);
    CHECK (OCR5A == 4800);
    CHECK (TCCR5A == (Mode14 | Connected));
}

TEST (servoNeedsATimer5Pin)
{
    adk::Servo servo {30};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::NotServo);
    CHECK (check::halted.pin == 30);
}

TEST (pwmCannotShareTimer5WithAServo)
{
    adk::Servo     servo  {44};
    adk::PwmOutput dimmer {45};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::TimerInUse);
    CHECK (adk::faultPin () == 45);
}

TEST (servoCannotTakeTimer5FromPwm)
{
    adk::PwmOutput dimmer {46};
    adk::Servo     servo  {44};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::TimerInUse);
    CHECK (adk::faultPin () == 44);
}

TEST (servoPinCannotBeUsedTwice)
{
    adk::Servo first  {44};
    adk::Servo second {44};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::PinInUse);
    CHECK (adk::faultPin () == 44);
}

TEST (stoppedServoGoesLimpUntilWrittenAgain)
{
    adk::Servo a {44};
    adk::Servo b {46};

    adk::setup ();
    a.write (90);
    b.write (90);

    adk::stop ();
    CHECK (TCCR5A == Mode14);
    CHECK (arduino::pin (44).output == LOW);

    a.write (45);
    CHECK (TCCR5A == (Mode14 | _BV (COM5C1)));
    CHECK (OCR5C == 2016);
}

TEST (moveToGlidesAtASteadyPaceAndArrivesOnTime)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.write (0);
    servo.moveTo (180, 1000);
    CHECK (servo.isMoving ());

    adk::update (5000);
    CHECK (OCR5C == 1088);

    adk::update (5250);
    CHECK (OCR5C == 2016);
    CHECK (servo.angle () == 45);

    adk::update (5500);
    CHECK (OCR5C == 2944);

    adk::update (5999);
    CHECK (OCR5C == 4796);
    CHECK (servo.isMoving ());

    adk::update (6000);
    CHECK (OCR5C == 4800);
    CHECK (servo.angle () == 180);
    CHECK (!servo.isMoving ());
}

TEST (moveToGlidesDownwardsToo)
{
    adk::Servo servo {45};

    adk::setup ();
    servo.write (180);
    servo.moveTo (0, 400);

    adk::update (0);
    adk::update (100);
    CHECK (OCR5B == 3872);

    adk::update (400);
    CHECK (OCR5B == 1088);
    CHECK (!servo.isMoving ());
}

TEST (longGlidesStayOnCourse)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.write (0);
    servo.moveTo (180, 100000);

    adk::update (0);
    adk::update (50000);
    CHECK (OCR5C == 2944);

    adk::update (99999);
    CHECK (servo.isMoving ());

    adk::update (100000);
    CHECK (OCR5C == 4800);
}

TEST (askingForTheSameGlideAgainKeepsItsPace)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.write (0);
    servo.moveTo (180, 1000);
    adk::update (0);
    adk::update (500);

    servo.moveTo (180, 1000);
    adk::update (1000);

    CHECK (OCR5C == 4800);
    CHECK (!servo.isMoving ());
}

TEST (limpServoMovesStraightToItsFirstAngle)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.moveTo (45, 1000);

    CHECK (!servo.isMoving ());
    CHECK (OCR5C == 2016);
    CHECK (TCCR5A == (Mode14 | _BV (COM5C1)));
}

TEST (writeOrStopEndsAGlide)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.write (0);
    servo.moveTo (180, 1000);
    adk::update (0);

    servo.write (10);
    adk::update (500);
    CHECK (!servo.isMoving ());
    CHECK (servo.angle () == 10);

    servo.moveTo (180, 1000);
    adk::update (600);
    adk::stop ();
    adk::update (1000);
    CHECK (!servo.isMoving ());
    CHECK (servo.angle () == 10);
}

TEST (stopLetsThePulseUnderWayFinishFirst)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.write (90);

    // Timer 5 counts half microseconds; 400 counts into a period, the
    // widest pulse, 2400 us, could still have 2200 us to go.
    TCNT5 = 400;
    unsigned long before = arduino::now ();
    adk::stop ();

    CHECK (TCCR5A == Mode14);
    CHECK (arduino::now () - before >= 2200);
    CHECK (arduino::now () - before <= 2210);
}

TEST (stopAfterThePulseDisconnectsAtOnce)
{
    adk::Servo servo {44};

    adk::setup ();
    servo.write (90);

    TCNT5 = 10000;
    unsigned long before = arduino::now ();
    adk::stop ();

    CHECK (TCCR5A == Mode14);
    CHECK (arduino::now () == before);
}

TEST (stopNearTheEndOfAPeriodWaitsOutTheNextPulse)
{
    adk::Servo servo {45, 1000, 2000};

    adk::setup ();
    servo.write (90);

    // 1000 us before the period ends, then a pulse of up to 2000 us.
    TCNT5 = 38000;
    unsigned long before = arduino::now ();
    adk::stop ();

    CHECK (TCCR5A == Mode14);
    CHECK (arduino::now () - before >= 3000);
    CHECK (arduino::now () - before <= 3010);
}
