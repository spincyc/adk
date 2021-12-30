#pragma once

#include <pin.h>
#include <color.h>

namespace adk { namespace led {

    struct mono : digital::output
    {
        using digital::output::output;

        void on();
        void off();
    };

    struct rgb
    {
        rgb(analog::output red_,analog::output green_,analog::output blue_);

        auto& red   () const { return _red   ; }
        auto& green () const { return _green ; }
        auto& blue  () const { return _blue  ; }

        void on(const color::rgb&);
        void off();

      protected:
        analog::output _red;
        analog::output _green;
        analog::output _blue;
    };

}}
