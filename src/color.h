#pragma once

#include <stdint.h>

namespace adk { namespace color {

    struct Rgb
    {
        using Raw = uint8_t;
        
        Rgb (Raw red, Raw green, Raw blue);

        bool operator== (const Rgb& other) const;
        bool operator!= (const Rgb& other) const;

        Raw red   () const;
        Raw green () const;
        Raw blue  () const;

      private:
        Raw red_;
        Raw green_;
        Raw blue_;
    };

    Rgb off    ();
    Rgb red    ();
    Rgb green  ();
    Rgb blue   ();
    Rgb orange ();

}}
