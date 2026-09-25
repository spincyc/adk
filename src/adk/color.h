#pragma once

#include <stdint.h>

namespace adk {

    // A color as red, green and blue brightness, each 0-255.
    struct Color
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    bool operator== (Color left, Color right);
    bool operator!= (Color left, Color right);

    // The color step/steps of the way from one color to another.
    Color blend (Color from, Color to, uint16_t step, uint16_t steps);

    // A color around the color wheel: 0 is red, 85 green, 170 blue.
    Color wheel (uint8_t position);

}

namespace adk::color {

    inline constexpr Color off     {0,   0,   0};
    inline constexpr Color white   {255, 255, 255};
    inline constexpr Color red     {255, 0,   0};
    inline constexpr Color orange  {255, 64,  0};
    inline constexpr Color yellow  {255, 160, 0};
    inline constexpr Color green   {0,   255, 0};
    inline constexpr Color cyan    {0,   255, 255};
    inline constexpr Color blue    {0,   0,   255};
    inline constexpr Color purple  {128, 0,   255};
    inline constexpr Color magenta {255, 0,   255};
    inline constexpr Color pink    {255, 64,  128};
}
