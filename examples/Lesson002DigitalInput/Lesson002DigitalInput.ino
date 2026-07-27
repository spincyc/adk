#include <Adk.h>

constexpr adk::PinId buttonPin = 22;

adk::Runtime      runtime;
adk::DigitalInput buttonInput (runtime.resources (), buttonPin, adk::Pull::Up);
adk::MonoLed      led         (runtime.resources (), LED_BUILTIN);

void setup ()
{
    buttonInput.initialize ();
    led.initialize         ();
}

void loop ()
{
    buttonInput.update ();

    const bool pressed = buttonInput.read () == adk::Level::Low;
    led.set                               (pressed);
}
