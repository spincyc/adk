#include "color.h"

namespace adk {

    namespace {

        uint8_t blendChannel (uint8_t from, uint8_t to, uint16_t step, uint16_t steps)
        {
            int32_t difference = static_cast<int32_t> (to) - from;
            return static_cast<uint8_t> (from + difference * step / steps);
        }
    }

    bool operator== (Color left, Color right)
    {
        return left.red   == right.red
            && left.green == right.green
            && left.blue  == right.blue;
    }

    bool operator!= (Color left, Color right)
    {
        return !(left == right);
    }

    Color blend (Color from, Color to, uint16_t step, uint16_t steps)
    {
        if (steps == 0 || step >= steps)
        {
            return to;
        }

        return {blendChannel (from.red,   to.red,   step, steps),
                blendChannel (from.green, to.green, step, steps),
                blendChannel (from.blue,  to.blue,  step, steps)};
    }

    Color wheel (uint8_t position)
    {
        // Three thirds: red to green, green to blue, blue back to red, so
        // 255 comes round to red again.
        uint8_t third = position < 85 ? 0 : position < 170 ? 1 : 2;
        uint8_t step  = static_cast<uint8_t> ((position - third * 85) * 3);
        uint8_t fall  = static_cast<uint8_t> (255 - step);

        switch (third)
        {
            case 0:  return {fall, step, 0};
            case 1:  return {0, fall, step};
            default: return {step, 0, fall};
        }
    }
}
