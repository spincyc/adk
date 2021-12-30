
#include <led.h>

namespace adk { namespace led {

    void mono::on()
    {
        write(true);
    }

    void mono::off()
    {
        write(false);
    }

    rgb::rgb(analog::output red_,analog::output green_,analog::output blue_)
        : _red  (red_)
        , _green(green_)
        , _blue (blue_)
    {
    }

    void rgb::on(const color::rgb& color_)
    {
        _red  .write(color_.red  ());
        _green.write(color_.green());
        _blue .write(color_.blue ());
    }

    void rgb::off()
    {
        on(color::rgb());
    }

}}
