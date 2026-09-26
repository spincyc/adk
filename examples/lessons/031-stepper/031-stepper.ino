// Lesson 31: Stepper
// Each press of the button turns the motor's shaft a quarter turn.

#include <Adk.h>

adk::Stepper motor  {A8, A9, A10, A11};
adk::Button  button {22};

constexpr long quarterTurn = adk::Stepper::StepsPerRevolution / 4;

void setup ()
{
    adk::setup ();
    motor.speed (500);
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        motor.step (quarterTurn);
    }
}
