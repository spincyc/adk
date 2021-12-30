
#include <pin.h>

namespace adk { namespace pin {

    base::base(id pin_)
        : _pin(pin_)
    {
    }

    void input::setup()
    {
        pinMode(pin(),INPUT);
    }

    void output::setup()
    {
        pinMode(pin(),OUTPUT);
    }

}}

namespace adk { namespace analog {

    pin::value input::read()
    {
        auto ret = analogRead(pin());
        return ret;
    }

    void output::write(pin::value v_)
    {
        analogWrite(pin(),v_);
    }

}}

namespace adk { namespace digital {

    bool input::read()
    {
        auto value = digitalRead(pin());
        return value == HIGH ? true : false;
    }

    void input_up::setup()
    {
        pinMode(pin(),INPUT_PULLUP);
    }

    void output::write(bool on_)
    {
        digitalWrite(pin(),on_ ? HIGH : LOW);
    }

}}

