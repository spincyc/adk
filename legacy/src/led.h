#pragma once

#include <pin.h>
#include <color.h>

namespace adk { namespace led {

    struct Mono : digital::Output
    {
        using digital::Output::Output;

        void on  () const;
        void off () const;
    };

    struct Rgb
    {
        Rgb (pin::Id redPin, pin::Id greenPin, pin::Id bluePin);

        const analog::Output& red   () const;
        const analog::Output& green () const;
        const analog::Output& blue  () const;

        void on  (const color::Rgb& color) const;
        void off () const;

      private:
        analog::Output red_;
        analog::Output green_;
        analog::Output blue_;
    };

}}
