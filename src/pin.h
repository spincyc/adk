#pragma once

#include <object.h>

#include <stdint.h>

namespace adk { namespace pin {

    using id    = uint8_t;
    using value = int32_t;

    struct base : object
    {
        base(id pin_);

        id pin() const { return _pin; }

      protected:
        id _pin;
    };

    struct input : base
    {
        using base::base;

      protected:
        void setup() override;
    };

    struct output : base
    {
        using base::base;

      protected:
        void setup() override;
    };

}}

namespace adk { namespace analog {

    struct input : pin::input
    {
        using pin::input::input;

        pin::value read();
    };

    struct output : pin::output
    {
        using pin::output::output;

        void write(pin::value);
    };

}}

namespace adk { namespace digital {

    struct input : pin::input
    {
        using pin::input::input;

        bool read();
    };

    struct input_up : input
    {
        using input::input;

      protected:
        void setup() override;
    };

    struct output : pin::output
    {
        using pin::output::output;

        void write(bool);
    };

}}
