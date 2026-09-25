#pragma once

#include "object.h"

#include <Arduino.h>

namespace adk {

    // A date and a time of day on the 24-hour clock, such as
    // {2026, 9, 24, 20, 30, 0} for 8:30 pm on 24 September 2026.
    struct DateTime
    {
        uint16_t year;
        uint8_t  month;
        uint8_t  day;
        uint8_t  hour;
        uint8_t  minute;
        uint8_t  second;
    };

    // A DS1307 real-time clock module, which keeps the date and time while
    // the Mega is off. A DS3231 module has the same time registers and works
    // too. It goes on the I2C bus:
    //
    //   SDA -> pin 20, SCL -> pin 21, VCC -> 5 V, GND -> GND
    //
    // A CR2032 coin cell keeps the time while the module is unplugged. Some
    // modules (the Tiny RTC, and the ZS-042 with a DS3231) charge their cell:
    // fit those with a rechargeable LIR2032, never a CR2032.
    //
    // The chip answers at I2C address 0x68, as does an MPU-6050 unless its
    // AD0 pin is raised. A missing chip is not a pin fault: the sketch runs,
    // and ok () is false.
    struct Rtc : Object
    {
        Rtc ();

        // Read the date and time from the chip, a transfer of about 1 ms, so
        // read it once per loop at most. All zeros if the chip does not
        // answer.
        DateTime now ();

        // Set the date and time, for years 2000-2099. This also starts a
        // halted clock.
        void set (DateTime time);

        // False when the clock is halted: a brand-new chip, or one whose cell
        // went flat. A DS3231 cannot halt, so it always reports running.
        bool isRunning ();

        // The chip answered the last time it was asked, at setup or since.
        bool ok () const;

      protected:
        void setup () override;

      private:
        bool ok_;
    };

    // Parse a date and time written as __DATE__ ("Sep 24 2026") and __TIME__
    // ("20:30:00") are, both in flash: compiledAt (F ("..."), F ("...")).
    DateTime compiledAt (const __FlashStringHelper* date, const __FlashStringHelper* time);

    // When the sketch was compiled, a few seconds before it was uploaded:
    // rtc.set (adk::compiledAt ()) sets the clock to about the right time.
    // Unlike the rest of the library it is defined in the header, because
    // __DATE__ and __TIME__ are fixed where the code is compiled, and here
    // that is the sketch. It is static so that each file calling it gets the
    // time that file was compiled, instead of the linker keeping one copy.
    static inline DateTime compiledAt ()
    {
        return compiledAt (F (__DATE__), F (__TIME__));
    }
}
