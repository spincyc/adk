#include <Adk.h>

constexpr adk::PinId buttonPin = 22;

adk::Runtime      runtime;
adk::DigitalInput buttonInput (runtime.resources (), buttonPin, adk::Pull::Up);
adk::MonoLed      led         (runtime.resources (), LED_BUILTIN);
bool              ready       = false;

void setup ()
{
    const adk::Status inputStatus = buttonInput.initialize ();
    const adk::Status ledStatus   = led.initialize         ();

    ready = inputStatus.ok () && ledStatus.ok ();

    if (!ready)
    {
        led.shutdown         ();
        buttonInput.shutdown ();
    }
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    buttonInput.update ();

    const bool pressed = buttonInput.read () == adk::Level::Low;

    const adk::Status outputStatus = led.set (pressed);

    if (!outputStatus.ok ())
    {
        led.shutdown         ();
        buttonInput.shutdown ();
        ready = false;
    }
}
