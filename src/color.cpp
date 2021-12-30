
#include "color.h"

namespace adk { namespace color {

    rgb::rgb()
        : _total(0)
    {
    }

    rgb::rgb(raw red_,raw green_,raw blue_)
        : rgb()
    {
        _color.red   = red_  ;
        _color.green = green_;
        _color.blue  = blue_ ;
    }

    bool rgb::operator==(const rgb& rhs_) const
    {
        return _total == rhs_._total;
    }

    rgb red()
    {
        return rgb(255,0,0);
    }

    rgb green()
    {
        return rgb(0,255,0);
    }

    rgb blue()
    {
        return rgb(0,0,255);
    }

    rgb orange()
    {
        return rgb(255,128,0);
    }

}}

#if 0

#include <iostream>

std::ostream& operator<<(std::ostream& os_,const arduino::color::rgb& rgb_)
{
    return os_ << '(' << rgb_.red()
               << ',' << rgb_.green()
               << ',' << rgb_.blue()
               << ')';
}

int main()
{
    arduino::color::rgb color;

    for(int i = 0; i < 10000; ++i)
    {
        std::cout << ++color << std::endl;
    }
}

#endif
