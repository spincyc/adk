#pragma once

#include <object.h>

#include <stdint.h>

namespace adk { namespace pin {

    using Id    = uint8_t;
    using Value = int32_t;

    struct Base : Object
    {
        explicit Base (Id pin);

        Id pin () const;

      protected:
        Id pin_;
    };

    struct Input : Base
    {
        using Base::Base;

      protected:
        void initialize () override;
    };

    struct Output : Base
    {
        using Base::Base;

      protected:
        void initialize () override;
    };

}}

namespace adk { namespace analog {

    struct Input : pin::Input
    {
        using pin::Input::Input;

        pin::Value read () const;
    };

    struct Output : pin::Output
    {
        using pin::Output::Output;

        void write (pin::Value value) const;
    };

}}

namespace adk { namespace digital {

    struct Input : pin::Input
    {
        using pin::Input::Input;

        bool read () const;
    };

    struct InputPullUp : Input
    {
        using Input::Input;

      protected:
        void initialize () override;
    };

    struct Output : pin::Output
    {
        using pin::Output::Output;

        void write (bool active) const;
    };

}}
