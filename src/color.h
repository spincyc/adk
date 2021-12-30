#pragma once

#include <stdint.h>

namespace adk { namespace color {

    struct rgb
    {
        using raw = uint8_t;
        
        rgb();
        rgb(raw red_,raw green_,raw blue_);

        bool operator==(const rgb&) const;

        raw red  () const { return _color.red  ; }
        raw green() const { return _color.green; }
        raw blue () const { return _color.blue ; }

      protected:
        struct color
        {
            raw red;
            raw green;
            raw blue;
        };

        union
        {
            uint32_t _total;
            color    _color;
        };
    };

    rgb red   ();
    rgb green ();
    rgb blue  ();
    rgb orange();

}}
