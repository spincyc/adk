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
        constexpr uint8_t Runs    = 99;         // start numbers go round from 1 to 99
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

        // A start number, 0 to 255.
        bool readRun (const char*& text, uint8_t& run)
        {
            long number = 0;

            if (*text < '0' || *text > '9' || !readNumber (text, number) || number > 255)
            {
                return false;
            }

            run = static_cast<uint8_t> (number);
            return true;
        }

        // The start numbers at the front of a line, "1/2", if it has them:
        // a line typed by hand may not.
        bool readStart (const char*& text, uint8_t& run, uint8_t& echo, bool& found)
        {
            const char* end = text;

            while (*end != '\0' && *end != ' ' && *end != '=')
            {
                ++end;
            }

            found = *end != '=' && memchr (text, '/', static_cast<size_t> (end - text));

            if (!found)
            {
                return true;
            }

            return readRun (text, run) && *text++ == '/' && readRun (text, echo)
                   && (*text == '\0' || *text == ' ');
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
        , next_        (0)
        , run_         (0)
        , theirRun_    (0)
        , beat_        (true)
        , heard_       (false)
        , stopped_     (false)
    {
    }

    void Bridge::share (const char* name, long value)
    {
        keep (name, value, 0, false);
    }

    void Bridge::shareEvent (const char* name, long count, long payload)
    {
        keep (name, count, payload, true);
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
            mine_[mineCount_++] = {name, value, payload, 0, event, true};
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

    void Bridge::stop ()
    {
        stopped_ = true;
    }

    void Bridge::start ()
    {
        stopped_ = false;
        beat_    = true;

        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            mine_[index].unsent = true;
        }
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
            hear (line + 1);
        }

        // Forget the other board once it is gone, so that 49.7 days of
        // silence can't bring millis () round to look like a recent message.
        if (heard_ && now - heardAt_ >= Silence)
        {
            heard_ = false;
        }

        if (!stopped_ && now - sentAt_ >= Pace)
        {
            tell (now);
        }
    }

    // A line is the start numbers, then tokens. A token is a value,
    // "angle=90", or an event, "key=3:7". Validate the whole token before
    // changing either number. A malformed token ends the reading; earlier
    // complete tokens remain valid.
    void Bridge::hear (const char* text)
    {
        uint8_t run    = 0;
        uint8_t echo   = 0;
        bool    tagged = false;

        if (!readStart (text, run, echo, tagged))
        {
            return;
        }

        if (tagged)
        {
            meet (run, echo);
        }

        // Sent before the other board heard this one's new start, it was
        // meant for this board as it was before it restarted. Sent before
        // the other board heard this one at all, its values stand, but its
        // events happened before the two boards found each other.
        if (tagged && echo != 0 && echo != run_)
        {
            return;
        }

        bool events = !tagged || echo != 0;

        heard_   = true;
        heardAt_ = now_;

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

            if (event && !events)
            {
                text = end;
                continue;
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

            // A value is news the first time it arrives; an event only once
            // its count goes up. Nothing is news to a stopped bridge.
            if (!theirs && theirsCount_ < MaxValues)
            {
                theirs = &theirs_[theirsCount_++];
                memcpy (theirs->name, text, length);
                theirs->name[length] = '\0';
                theirs->value        = 0;
                theirs->payload      = 0;
                theirs->event        = event;
                theirs->fresh        = !event;
            }

            if (theirs)
            {
                long before = theirs->event ? theirs->value : 0;
                bool news   = event ? number > before : theirs->event || theirs->value != number;

                theirs->fresh   = (theirs->fresh || news) && !stopped_;
                theirs->value   = number;
                theirs->payload = payload;
                theirs->event   = event;
            }

            text = end;
        }
    }

    // This board takes its start number from the first line it hears: one
    // more than the other board remembers for it. That is usually a new
    // number, but not always: if this board restarted and spoke first, the
    // other heard 0 and offers 1 again. Either way the other board sees a
    // change, 0 in between, so it knows they have just found each other.
    void Bridge::meet (uint8_t run, uint8_t echo)
    {
        bool met = false;

        if (run_ == 0)
        {
            run_ = static_cast<uint8_t> (echo % Runs + 1);
            met  = true;
        }

        if (run != theirRun_)
        {
            theirRun_ = run;
            met       = true;
        }

        if (met)
        {
            restart ();
        }
    }

    // Count events afresh from here, both ways: the other board's start
    // numbers have changed, or this board's, so the other does the same.
    // And send everything at once.
    void Bridge::restart ()
    {
        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            mine_[index].start  = mine_[index].value;
            mine_[index].unsent = true;
        }

        for (uint8_t index = 0; index < theirsCount_; ++index)
        {
            if (theirs_[index].event)
            {
                theirs_[index].value = 0;
            }
        }

        beat_ = true;
    }

    // As many unsent values as fit in one line. Every two seconds they all
    // count as unsent again, and a board with nothing to share sends just
    // its start numbers, so the other still hears from it.
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
        bool          any                = false;
        uint8_t       next               = next_;

        print (line, Mark, static_cast<int> (run_), '/', static_cast<int> (theirRun_));

        for (uint8_t offset = 0; offset < mineCount_; ++offset)
        {
            uint8_t     index = static_cast<uint8_t> ((next_ + offset) % mineCount_);
            const Mine& mine = mine_[index];
            Text<32>    pair;

            if (!mine.unsent)
            {
                continue;
            }

            if (mine.event)
            {
                // Round the 32 bits, as the Mega's own long would.
                int32_t count = static_cast<int32_t> (static_cast<uint32_t> (mine.value)
                                                      - static_cast<uint32_t> (mine.start));
                print (pair, mine.name, '=', static_cast<long> (count), ':', mine.payload);
            }
            else
            {
                print (pair, mine.name, '=', mine.value);
            }

            // Leave the next unsent value first in line for the next
            // packet, even if a smaller value would still fit this one.
            if (line.size () + 1 + pair.size () > Longest)
            {
                break;
            }

            print (line, ' ', pair.c_str ());
            chosen[index] = true;
            any           = true;
            next          = static_cast<uint8_t> ((index + 1) % mineCount_);
        }

        if ((!any && !beat_) || !out_.sendLine (line.c_str ()))
        {
            return;
        }

        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            mine_[index].unsent = mine_[index].unsent && !chosen[index];
        }

        sentAt_ = now;
        beat_   = false;
        next_   = next;
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
