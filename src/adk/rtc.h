#pragma once

#include "object.h"
#include "timing.h"

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
    // A coin cell keeps the time while the module is unplugged: a CR1220 on
    // the kit's module (Elegoo's DS1307-Module-V03), a CR2032 on some. Some
    // modules (the Tiny RTC, and the ZS-042 with a DS3231) charge their cell:
    // fit those with a rechargeable LIR2032, never a CR2032.
    //
    // The chip answers at I2C address 0x68, as does an MPU-6050 unless its
    // AD0 pin is raised. A missing chip is not a pin fault: the sketch runs,
    // and ok () is false.
    //
    // The clock reads the chip at setup () and then ten times a second, in
    // update (), each a transfer of about 1 ms. now () and isRunning () give
    // the latest reading, so they may be asked as often as a sketch likes.
    struct Rtc : Object
    {
        Rtc ();

        // The date and time, as of the latest reading: no more than a tenth
        // of a second old while loop () keeps updating. All zeros if the
        // chip did not answer.
        DateTime now () const;

        // Set the date and time, for years 2000-2099, at once. This also
        // starts a halted clock, and now () gives the new time straight away.
        void set (DateTime time);

        // False when the clock is halted: a brand-new chip, or one whose cell
        // went flat. A DS3231 cannot halt, so it always reports running.
        bool isRunning () const;

        // The chip answered when it was last read or set.
        bool ok () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        void read ();

        StartTime read_;
        DateTime  time_;
        bool      running_;
        bool      ok_;
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
