
#include "color.h"

namespace adk { namespace color {

    Rgb::Rgb (Raw red, Raw green, Raw blue)
        : red_   (red)
        , green_ (green)
        , blue_  (blue)
    {
    }

    bool Rgb::operator==(const Rgb& other) const
    {
        return red_   == other.red_
            && green_ == other.green_
            && blue_  == other.blue_;
    }

    bool Rgb::operator!=(const Rgb& other) const
    {
        return !(*this == other);
    }

    Rgb::Raw Rgb::red () const
    {
        return red_;
    }

    Rgb::Raw Rgb::green () const
    {
        return green_;
    }

    Rgb::Raw Rgb::blue () const
    {
        return blue_;
    }

    Rgb off ()
    {
        return Rgb (0, 0, 0);
    }

    Rgb red ()
    {
        return Rgb (255, 0, 0);
    }

    Rgb green ()
    {
        return Rgb (0, 255, 0);
    }

    Rgb blue ()
    {
        return Rgb (0, 0, 255);
    }

    Rgb orange ()
    {
        return Rgb (255, 128, 0);
    }

}}
