#include "bridge.h"

#include "print.h"

#include <string.h>

namespace adk {

    namespace {

        constexpr Millis  Pace    = 100;        // at most ten messages a second
        constexpr Millis  Refresh = 2000;       // everything, again
        constexpr Millis  Silence = 5000;       // after which the other board is gone
        constexpr uint8_t Longest = 56;         // the shortest radio line, LoraLink's
        constexpr char    Mark    = '@';        // a bridge message, not stray text
        constexpr long    Lowest  = -2147483647L - 1;
        constexpr long    Highest = 2147483647L;

        // The wire carries a Mega's signed 32-bit long, including its minimum.
        // Bound each digit before multiplying, even on a host with wider longs.
        bool readNumber (const char*& text, long& number)
        {
            bool negative = *text == '-';

            if (negative || *text == '+')
            {
                ++text;
            }

            if (*text < '0' || *text > '9')
            {
                return false;
            }

            uint32_t magnitude = 0;
            uint32_t limit     = negative ? 2147483648UL : 2147483647UL;

            while (*text >= '0' && *text <= '9')
            {
                uint8_t digit = static_cast<uint8_t> (*text - '0');

                if (magnitude > (limit - digit) / 10)
                {
                    return false;
                }

                magnitude = magnitude * 10 + digit;
                ++text;
            }

            number = magnitude == 2147483648UL ? Lowest : static_cast<long> (magnitude);

            if (negative && magnitude != 2147483648UL)
            {
                number = -number;
            }

            return true;
        }
    }

    Bridge::Bridge (Link& radio)
        : Bridge (radio, radio)
    {
    }

    Bridge::Bridge (Link& out, Link& in)
        : out_         (out)
        , in_          (in)
        , mine_        {}
        , theirs_      {}
        , now_         (0)
        , sentAt_      (0)
        , refreshedAt_ (0)
        , heardAt_     (0)
        , mineCount_   (0)
        , theirsCount_ (0)
        , beat_        (false)
        , heard_       (false)
    {
    }

    void Bridge::share (const char* name, long value)
    {
        keep (name, value, 0, false);
    }

    void Bridge::shareEvent (const char* name, long sequence, long payload)
    {
        keep (name, sequence, payload, true);
    }

    void Bridge::keep (const char* name, long value, long payload, bool event)
    {
        if (value < Lowest || value > Highest || payload < Lowest || payload > Highest)
        {
            return;
        }

        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            Mine& mine = mine_[index];

            if (strcmp (mine.name, name) == 0)
            {
                mine.unsent  = mine.unsent || mine.value != value || mine.payload != payload
                               || mine.event != event;
                mine.value   = value;
                mine.payload = payload;
                mine.event   = event;
                return;
            }
        }

        if (mineCount_ < MaxValues && strlen (name) <= NameLength)
        {
            mine_[mineCount_++] = {name, value, payload, event, true};
        }
    }

    long Bridge::value (const char* name) const
    {
        const Theirs* theirs = find (name);
        return theirs ? theirs->value : 0;
    }

    long Bridge::payload (const char* name) const
    {
        const Theirs* theirs = find (name);
        return theirs ? theirs->payload : 0;
    }

    bool Bridge::changed (const char* name) const
    {
        const Theirs* theirs = find (name);
        return theirs && theirs->fresh;
    }

    bool Bridge::isConnected () const
    {
        return heard_ && now_ - heardAt_ < Silence;
    }

    void Bridge::update (Millis now)
    {
        now_ = now;

        for (uint8_t index = 0; index < theirsCount_; ++index)
        {
            theirs_[index].fresh = false;
        }

        const char* line = in_.heardLine ();

        if (line && line[0] == Mark)
        {
            heard_   = true;
            heardAt_ = now;
            hear (line + 1);
        }

        if (now - sentAt_ >= Pace)
        {
            tell (now);
        }
    }

    // A token is a scalar, "angle=90", or an event, "key=3:7". Validate
    // the whole token before changing either number. A malformed token ends
    // the reading; earlier complete tokens remain valid.
    void Bridge::hear (const char* text)
    {
        while (*text != '\0')
        {
            while (*text == ' ')
            {
                ++text;
            }

            const char* equals = text;

            while (*equals != '\0' && *equals != ' ' && *equals != '=')
            {
                ++equals;
            }

            if (*equals != '=')
            {
                return;
            }

            size_t      length  = static_cast<size_t> (equals - text);
            const char* end     = equals + 1;
            long        number  = 0;
            long        payload = 0;

            if (length == 0 || length > NameLength || !readNumber (end, number))
            {
                return;
            }

            bool event = *end == ':';

            if (event)
            {
                ++end;

                if (!readNumber (end, payload))
                {
                    return;
                }
            }

            if (*end != '\0' && *end != ' ')
            {
                return;
            }

            Theirs* theirs = nullptr;

            for (uint8_t index = 0; index < theirsCount_; ++index)
            {
                if (strlen (theirs_[index].name) == length
                    && strncmp (theirs_[index].name, text, length) == 0)
                {
                    theirs = &theirs_[index];
                }
            }

            if (!theirs && theirsCount_ < MaxValues)
            {
                theirs = &theirs_[theirsCount_++];
                memcpy (theirs->name, text, length);
                theirs->name[length] = '\0';
                theirs->fresh        = true;
            }

            if (theirs)
            {
                theirs->fresh   = theirs->fresh || theirs->value != number
                                  || theirs->payload != payload || theirs->event != event;
                theirs->value   = number;
                theirs->payload = payload;
                theirs->event   = event;
            }

            text = end;
        }
    }

    // As many unsent values as fit in one line. Every two seconds they all
    // count as unsent again, and a board with nothing to share sends just
    // the mark, so the other still hears from it.
    void Bridge::tell (Millis now)
    {
        if (now - refreshedAt_ >= Refresh)
        {
            refreshedAt_ = now;
            beat_        = true;

            for (uint8_t index = 0; index < mineCount_; ++index)
            {
                mine_[index].unsent = true;
            }
        }

        Text<Longest> line;
        bool          chosen [MaxValues] = {};

        print (line, Mark);

        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            const Mine& mine = mine_[index];
            Text<32>    pair;

            print (pair, mine.name, '=', mine.value);

            if (mine.event)
            {
                print (pair, ':', mine.payload);
            }

            size_t separator = line.size () > 1 ? 1 : 0;

            if (!mine.unsent || line.size () + separator + pair.size () > Longest)
            {
                continue;
            }

            if (line.size () > 1)
            {
                print (line, ' ');
            }

            print (line, pair.c_str ());
            chosen[index] = true;
        }

        if ((line.size () == 1 && !beat_) || !out_.sendLine (line.c_str ()))
        {
            return;
        }

        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            mine_[index].unsent = mine_[index].unsent && !chosen[index];
        }

        sentAt_ = now;
        beat_   = false;
    }

    const Bridge::Theirs* Bridge::find (const char* name) const
    {
        for (uint8_t index = 0; index < theirsCount_; ++index)
        {
            if (strcmp (theirs_[index].name, name) == 0)
            {
                return &theirs_[index];
            }
        }

        return nullptr;
    }
}
