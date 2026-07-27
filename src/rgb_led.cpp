#include "rgb_led.h"

namespace adk {

    Rgb::Rgb (uint8_t red, uint8_t green, uint8_t blue) noexcept
        : red_   (red)
        , green_ (green)
        , blue_  (blue)
    {
    }

    bool Rgb::operator== (const Rgb& other) const noexcept
    {
        return red_ == other.red_ && green_ == other.green_ &&
               blue_ == other.blue_;
    }

    bool Rgb::operator!= (const Rgb& other) const noexcept
    {
        return !(*this == other);
    }

    uint8_t Rgb::red () const noexcept
    {
        return red_;
    }

    uint8_t Rgb::green () const noexcept
    {
        return green_;
    }

    uint8_t Rgb::blue () const noexcept
    {
        return blue_;
    }

    RgbLed::RgbLed (ResourceRegistry&    resources,
                    const RgbLedChannel& red,
                    const RgbLedChannel& green,
                    const RgbLedChannel& blue) noexcept
        : redChannel_   (red)
        , greenChannel_ (green)
        , blueChannel_  (blue)
        , red_          (resources, red.pin)
        , green_        (resources, green.pin)
        , blue_         (resources, blue.pin)
        , color_        ()
    {
    }

    RgbLed::~RgbLed () noexcept
    {
        shutdown ();
    }

    Status RgbLed::initialize () noexcept
    {
        if (initialized ())
        {
            return Status::Ok;
        }

        if (redChannel_.resistorOhms == 0 ||
            greenChannel_.resistorOhms == 0 ||
            blueChannel_.resistorOhms == 0)
        {
            return Status::InvalidArgument;
        }

        Status status = red_.initialize ();

        if (status != Status::Ok)
        {
            return status;
        }

        status = green_.initialize ();

        if (status != Status::Ok)
        {
            red_.shutdown ();
            return status;
        }

        status = blue_.initialize ();

        if (status != Status::Ok)
        {
            green_.shutdown ();
            red_  .shutdown ();
            return status;
        }

        color_ = Rgb ();
        return Status::Ok;
    }

    void RgbLed::shutdown () noexcept
    {
        blue_ .shutdown ();
        green_.shutdown ();
        red_  .shutdown ();

        color_ = Rgb ();
    }

    Status RgbLed::set (const Rgb& color) noexcept
    {
        if (!initialized ())
        {
            return Status::NotInitialized;
        }

        const Rgb previous = color_;
        Status    status   = red_.write (color.red ());

        if (status != Status::Ok)
        {
            return status;
        }

        status = green_.write (color.green ());

        if (status != Status::Ok)
        {
            red_.write (previous.red ());
            return status;
        }

        status = blue_.write (color.blue ());

        if (status != Status::Ok)
        {
            green_.write (previous.green ());
            red_  .write (previous.red ());
            return status;
        }

        color_ = color;
        return Status::Ok;
    }

    Status RgbLed::off () noexcept
    {
        return set (Rgb ());
    }

    const Rgb& RgbLed::color () const noexcept
    {
        return color_;
    }

    bool RgbLed::initialized () const noexcept
    {
        return red_.initialized () && green_.initialized () &&
               blue_.initialized ();
    }

    const RgbLedChannel& RgbLed::redChannel () const noexcept
    {
        return redChannel_;
    }

    const RgbLedChannel& RgbLed::greenChannel () const noexcept
    {
        return greenChannel_;
    }

    const RgbLedChannel& RgbLed::blueChannel () const noexcept
    {
        return blueChannel_;
    }
}
