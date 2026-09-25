#pragma once

#include <stdint.h>

namespace adk::note {

    // Equal-tempered pitches in hertz, a4 = 440 Hz; s means sharp, so cs4 is
    // C#4. The names are lowercase because the AVR headers already use names
    // like AS2 for register bits.

    inline constexpr uint16_t c2  = 65;
    inline constexpr uint16_t cs2 = 69;
    inline constexpr uint16_t d2  = 73;
    inline constexpr uint16_t ds2 = 78;
    inline constexpr uint16_t e2  = 82;
    inline constexpr uint16_t f2  = 87;
    inline constexpr uint16_t fs2 = 92;
    inline constexpr uint16_t g2  = 98;
    inline constexpr uint16_t gs2 = 104;
    inline constexpr uint16_t a2  = 110;
    inline constexpr uint16_t as2 = 117;
    inline constexpr uint16_t b2  = 123;

    inline constexpr uint16_t c3  = 131;
    inline constexpr uint16_t cs3 = 139;
    inline constexpr uint16_t d3  = 147;
    inline constexpr uint16_t ds3 = 156;
    inline constexpr uint16_t e3  = 165;
    inline constexpr uint16_t f3  = 175;
    inline constexpr uint16_t fs3 = 185;
    inline constexpr uint16_t g3  = 196;
    inline constexpr uint16_t gs3 = 208;
    inline constexpr uint16_t a3  = 220;
    inline constexpr uint16_t as3 = 233;
    inline constexpr uint16_t b3  = 247;

    inline constexpr uint16_t c4  = 262;
    inline constexpr uint16_t cs4 = 277;
    inline constexpr uint16_t d4  = 294;
    inline constexpr uint16_t ds4 = 311;
    inline constexpr uint16_t e4  = 330;
    inline constexpr uint16_t f4  = 349;
    inline constexpr uint16_t fs4 = 370;
    inline constexpr uint16_t g4  = 392;
    inline constexpr uint16_t gs4 = 415;
    inline constexpr uint16_t a4  = 440;
    inline constexpr uint16_t as4 = 466;
    inline constexpr uint16_t b4  = 494;

    inline constexpr uint16_t c5  = 523;
    inline constexpr uint16_t cs5 = 554;
    inline constexpr uint16_t d5  = 587;
    inline constexpr uint16_t ds5 = 622;
    inline constexpr uint16_t e5  = 659;
    inline constexpr uint16_t f5  = 698;
    inline constexpr uint16_t fs5 = 740;
    inline constexpr uint16_t g5  = 784;
    inline constexpr uint16_t gs5 = 831;
    inline constexpr uint16_t a5  = 880;
    inline constexpr uint16_t as5 = 932;
    inline constexpr uint16_t b5  = 988;

    inline constexpr uint16_t c6  = 1047;
    inline constexpr uint16_t cs6 = 1109;
    inline constexpr uint16_t d6  = 1175;
    inline constexpr uint16_t ds6 = 1245;
    inline constexpr uint16_t e6  = 1319;
    inline constexpr uint16_t f6  = 1397;
    inline constexpr uint16_t fs6 = 1480;
    inline constexpr uint16_t g6  = 1568;
    inline constexpr uint16_t gs6 = 1661;
    inline constexpr uint16_t a6  = 1760;
    inline constexpr uint16_t as6 = 1865;
    inline constexpr uint16_t b6  = 1976;

    inline constexpr uint16_t c7  = 2093;
    inline constexpr uint16_t cs7 = 2217;
    inline constexpr uint16_t d7  = 2349;
    inline constexpr uint16_t ds7 = 2489;
    inline constexpr uint16_t e7  = 2637;
    inline constexpr uint16_t f7  = 2794;
    inline constexpr uint16_t fs7 = 2960;
    inline constexpr uint16_t g7  = 3136;
    inline constexpr uint16_t gs7 = 3322;
    inline constexpr uint16_t a7  = 3520;
    inline constexpr uint16_t as7 = 3729;
    inline constexpr uint16_t b7  = 3951;

    inline constexpr uint16_t c8  = 4186;

    inline constexpr uint16_t rest = 0;
}
