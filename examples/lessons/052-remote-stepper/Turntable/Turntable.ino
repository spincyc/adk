// Lesson 52: Remote Stepper, Board B
// The turntable turns to whatever angle Board A's knob asks for, and
// tells Board A where it has got to. The L LED lights while the bridge
// hears Board A.

#include <Adk.h>

adk::Stepper motor     {A8, A9, A10, A11};
adk::Led     connected {LED_BUILTIN};

// The bridge to Board A, over the LoRa modem on Serial3.
adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

// 4096 half-steps make a turn: about 11.4 of them to a degree.
constexpr float stepsPerDegree = adk::Stepper::StepsPerRevolution / 360.0;

void setup ()
{
    adk::setup ();
    motor.speed (500);
}

void loop ()
{
    adk::update ();

    long angle = bridge.value ("angle");
    motor.moveTo (lround (angle * stepsPerDegree));

    long at = lround (motor.position () / stepsPerDegree);
    bridge.share ("at", at);

    connected.set (bridge.isConnected ());
}
