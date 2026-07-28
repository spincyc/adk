#include <Adk.h>

constexpr adk::PinId buttonPin = 22;
constexpr unsigned long acquisitionPulseMilliseconds = 250;
constexpr unsigned long acquisitionGapMilliseconds = 750;
constexpr unsigned long shutdownLowMilliseconds = 250;
constexpr unsigned long shutdownHoldMilliseconds = 3000;

adk::Runtime      runtime;
adk::DigitalInput buttonInput (runtime.resources (), buttonPin, adk::Pull::Up);
adk::MonoLed      led         (runtime.resources (), LED_BUILTIN);
bool              ready       = false;
bool              holdActive  = false;
unsigned long     holdStarted = 0;

void enterPassiveState ();

void setup ()
{
    const adk::Status inputStatus = buttonInput.initialize ();
    const adk::Status ledStatus   = led.initialize         ();

    ready = inputStatus.ok () && ledStatus.ok ();

    if (!ready)
    {
        led.shutdown         ();
        buttonInput.shutdown ();
        return;
    }

    if (!led.on ().ok ())
    {
        enterPassiveState ();
        return;
    }

    delay (acquisitionPulseMilliseconds);

    if (!led.off ().ok ())
    {
        enterPassiveState ();
        return;
    }

    delay (acquisitionGapMilliseconds);
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    buttonInput.update ();

    const bool pressed = buttonInput.read () == adk::Level::Low;

    if (!pressed)
    {
        holdActive = false;
    }
    else if (!holdActive)
    {
        holdActive  = true;
        holdStarted = millis ();
    }
    else if (millis () - holdStarted >= shutdownHoldMilliseconds)
    {
        enterPassiveState ();
        return;
    }

    const adk::Status outputStatus = led.set (pressed);

    if (!outputStatus.ok ())
    {
        enterPassiveState ();
    }
}

void enterPassiveState ()
{
    led.off              ();
    delay                (shutdownLowMilliseconds);
    led.shutdown         ();
    buttonInput.shutdown ();
    ready      = false;
    holdActive = false;
}
