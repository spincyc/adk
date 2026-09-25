#pragma once

#include <stdint.h>

namespace adk { namespace note {

    // Equal-tempered pitches in hertz, a4 = 440 Hz; s means sharp, so cs4 is
    // C#4. The names are lowercase because the AVR headers already use names
    // like AS2 for register bits.

    constexpr uint16_t c2  = 65;
    constexpr uint16_t cs2 = 69;
    constexpr uint16_t d2  = 73;
    constexpr uint16_t ds2 = 78;
    constexpr uint16_t e2  = 82;
    constexpr uint16_t f2  = 87;
    constexpr uint16_t fs2 = 92;
    constexpr uint16_t g2  = 98;
    constexpr uint16_t gs2 = 104;
    constexpr uint16_t a2  = 110;
    constexpr uint16_t as2 = 117;
    constexpr uint16_t b2  = 123;

    constexpr uint16_t c3  = 131;
    constexpr uint16_t cs3 = 139;
    constexpr uint16_t d3  = 147;
    constexpr uint16_t ds3 = 156;
    constexpr uint16_t e3  = 165;
    constexpr uint16_t f3  = 175;
    constexpr uint16_t fs3 = 185;
    constexpr uint16_t g3  = 196;
    constexpr uint16_t gs3 = 208;
    constexpr uint16_t a3  = 220;
    constexpr uint16_t as3 = 233;
    constexpr uint16_t b3  = 247;

    constexpr uint16_t c4  = 262;
    constexpr uint16_t cs4 = 277;
    constexpr uint16_t d4  = 294;
    constexpr uint16_t ds4 = 311;
    constexpr uint16_t e4  = 330;
    constexpr uint16_t f4  = 349;
    constexpr uint16_t fs4 = 370;
    constexpr uint16_t g4  = 392;
    constexpr uint16_t gs4 = 415;
    constexpr uint16_t a4  = 440;
    constexpr uint16_t as4 = 466;
    constexpr uint16_t b4  = 494;

    constexpr uint16_t c5  = 523;
    constexpr uint16_t cs5 = 554;
    constexpr uint16_t d5  = 587;
    constexpr uint16_t ds5 = 622;
    constexpr uint16_t e5  = 659;
    constexpr uint16_t f5  = 698;
    constexpr uint16_t fs5 = 740;
    constexpr uint16_t g5  = 784;
    constexpr uint16_t gs5 = 831;
    constexpr uint16_t a5  = 880;
    constexpr uint16_t as5 = 932;
    constexpr uint16_t b5  = 988;

    constexpr uint16_t c6  = 1047;
    constexpr uint16_t cs6 = 1109;
    constexpr uint16_t d6  = 1175;
    constexpr uint16_t ds6 = 1245;
    constexpr uint16_t e6  = 1319;
    constexpr uint16_t f6  = 1397;
    constexpr uint16_t fs6 = 1480;
    constexpr uint16_t g6  = 1568;
    constexpr uint16_t gs6 = 1661;
    constexpr uint16_t a6  = 1760;
    constexpr uint16_t as6 = 1865;
    constexpr uint16_t b6  = 1976;

    constexpr uint16_t c7  = 2093;
    constexpr uint16_t cs7 = 2217;
    constexpr uint16_t d7  = 2349;
    constexpr uint16_t ds7 = 2489;
    constexpr uint16_t e7  = 2637;
    constexpr uint16_t f7  = 2794;
    constexpr uint16_t fs7 = 2960;
    constexpr uint16_t g7  = 3136;
    constexpr uint16_t gs7 = 3322;
    constexpr uint16_t a7  = 3520;
    constexpr uint16_t as7 = 3729;
    constexpr uint16_t b7  = 3951;

    constexpr uint16_t c8  = 4186;

    constexpr uint16_t rest = 0;
}}
