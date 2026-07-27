
#include <pin.h>

#include <Arduino.h>

namespace adk { namespace pin {

    Base::Base (Id pin)
        : pin_ (pin)
    {
    }

    Id Base::pin () const
    {
        return pin_;
    }

    void Input::initialize ()
    {
        pinMode (pin (), INPUT);
    }

    void Output::initialize ()
    {
        pinMode (pin (), OUTPUT);
    }

}}

namespace adk { namespace analog {

    pin::Value Input::read () const
    {
        return analogRead (pin ());
    }

    void Output::write (pin::Value value) const
    {
        analogWrite (pin (), value);
    }

}}

namespace adk { namespace digital {

    bool Input::read () const
    {
        auto value = digitalRead (pin ());
        return value == HIGH;
    }

    void InputPullUp::initialize ()
    {
        pinMode (pin (), INPUT_PULLUP);
    }

    void Output::write (bool active) const
    {
        digitalWrite (pin (), active ? HIGH : LOW);
    }

}}
