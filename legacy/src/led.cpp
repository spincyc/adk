
#include <led.h>

namespace adk { namespace led {

    void Mono::on () const
    {
        write (true);
    }

    void Mono::off () const
    {
        write (false);
    }

    Rgb::Rgb (pin::Id redPin, pin::Id greenPin, pin::Id bluePin)
        : red_   (redPin)
        , green_ (greenPin)
        , blue_  (bluePin)
    {
    }

    const analog::Output& Rgb::red () const
    {
        return red_;
    }

    const analog::Output& Rgb::green () const
    {
        return green_;
    }

    const analog::Output& Rgb::blue () const
    {
        return blue_;
    }

    void Rgb::on (const color::Rgb& color) const
    {
        red_  .write (color.red ());
        green_.write (color.green ());
        blue_ .write (color.blue ());
    }

    void Rgb::off () const
    {
        on (color::off ());
    }

}}
