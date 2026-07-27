#include <Adk.h>

adk::Runtime             runtime;
const adk::ButtonConfig  buttonConfig (
    22, adk::Pull::Up, adk::Level::Low, adk::Duration (20));
adk::Button              button (runtime.resources (), buttonConfig);
adk::MonoLed             led    (runtime.resources (), LED_BUILTIN);
adk::ReactionTimerConfig timerConfig;
adk::ReactionTimer       timer  (timerConfig);

void setup ()
{
    button.initialize ();
    led.initialize    ();
    timer.initialize  ();
}

void loop ()
{
    const adk::TimePoint now (millis ());

    button.update (now);
    timer.update  (now, button);
    led.set       (timer.snapshot ().ledOn);
}
