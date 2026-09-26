#include "rtc.h"

#include "i2c.h"

namespace adk {

    namespace {

        constexpr uint8_t Address = 0x68;

        // The time registers, from 0x00: seconds, minutes, hours, day of the
        // week, date, month, and year, each in binary-coded decimal.
        constexpr uint8_t Seconds    = 0x00;
        constexpr uint8_t ClockHalt  = 0x80;
        constexpr uint8_t TwelveHour = 0x40;
        constexpr uint8_t Afternoon  = 0x20;

        constexpr char MonthNames [] PROGMEM = "JanFebMarAprMayJunJulAugSepOctNovDec";

        uint8_t fromBcd (uint8_t bcd)
        {
            return static_cast<uint8_t> ((bcd >> 4) * 10 + (bcd & 0x0F));
        }

        uint8_t toBcd (uint8_t value)
        {
            return static_cast<uint8_t> (((value / 10) << 4) | (value % 10));
        }

        // The chip may have been set to the 12-hour clock by other software,
        // where 12 am is midnight and 12 pm noon.
        uint8_t hourOf (uint8_t hours)
        {
            if (!(hours & TwelveHour))
            {
                return fromBcd (hours & 0x3F);
            }

            uint8_t hour = fromBcd (hours & 0x1F) % 12;

            return (hours & Afternoon) ? static_cast<uint8_t> (hour + 12) : hour;
        }

        // The chip only counts the day of the week up, 1 to 7, at midnight.
        // Write it as ISO 8601 does, 1 for Monday to 7 for Sunday, so other
        // software reading the chip agrees. Zeller's congruence counts January
        // and February as months 13 and 14 of the year before, and gives 0
        // for Saturday.
        uint8_t weekdayOf (DateTime date)
        {
            unsigned year  = date.month < 3 ? date.year - 1u : date.year;
            unsigned month = date.month < 3 ? date.month + 12u : date.month;
            unsigned days  = date.day + 13 * (month + 1) / 5 + year;

            days += year / 4 - year / 100 + year / 400;
            return static_cast<uint8_t> ((days + 5) % 7 + 1);
        }

        // Two digits in flash; a space counts as 0, as in __DATE__ ("Sep  4").
        uint8_t twoDigits (const char* text)
        {
            uint8_t tens = pgm_read_byte (text);
            uint8_t ones = pgm_read_byte (text + 1);

            return static_cast<uint8_t> ((tens == ' ' ? 0 : tens - '0') * 10 + ones - '0');
        }

        uint8_t monthNamed (const char* name)
        {
            for (uint8_t month = 0; month < 12; ++month)
            {
                const char* candidate = MonthNames + 3 * month;
                uint8_t     matched   = 0;

                while (matched < 3
                       && pgm_read_byte (name + matched) == pgm_read_byte (candidate + matched))
                {
                    ++matched;
                }

                if (matched == 3)
                {
                    return static_cast<uint8_t> (month + 1);
                }
            }

            return 0;
        }
    }

    Rtc::Rtc ()
        : ok_ (false)
    {
    }

    void Rtc::setup ()
    {
        if (i2c::begin ())
        {
            ok_ = i2c::present (Address);
        }
    }

    DateTime Rtc::now ()
    {
        uint8_t registers [7];

        ok_ = i2c::read (Address, Seconds, registers, sizeof registers);

        if (!ok_)
        {
            return {};
        }

        // The top bit of the month is a DS3231's century flag.
        return {static_cast<uint16_t> (2000 + fromBcd (registers[6])),
                fromBcd (registers[5] & 0x1F),
                fromBcd (registers[4] & 0x3F),
                hourOf  (registers[2]),
                fromBcd (registers[1] & 0x7F),
                fromBcd (registers[0] & 0x7F)};
    }

    void Rtc::set (DateTime time)
    {
        // Seconds written with the halt bit clear start the clock, and the
        // hours are written for the 24-hour clock.
        const uint8_t registers [] = {Seconds,
                                      toBcd     (time.second),
                                      toBcd     (time.minute),
                                      toBcd     (time.hour),
                                      weekdayOf (time),
                                      toBcd     (time.day),
                                      toBcd     (time.month),
                                      toBcd     (static_cast<uint8_t> (time.year % 100))};

        ok_ = i2c::write (Address, registers, sizeof registers);
    }

    bool Rtc::isRunning ()
    {
        uint8_t seconds = ClockHalt;

        ok_ = i2c::read (Address, Seconds, &seconds, 1);
        return ok_ && !(seconds & ClockHalt);
    }

    bool Rtc::ok () const
    {
        return ok_;
    }

    DateTime compiledAt (const __FlashStringHelper* date, const __FlashStringHelper* time)
    {
        // "Sep 24 2026" and "20:30:00"
        const char* day  = reinterpret_cast<const char*> (date);
        const char* hour = reinterpret_cast<const char*> (time);

        return {static_cast<uint16_t> (twoDigits (day + 7) * 100 + twoDigits (day + 9)),
                monthNamed (day),
                twoDigits  (day + 4),
                twoDigits  (hour),
                twoDigits  (hour + 3),
                twoDigits  (hour + 6)};
    }
}
