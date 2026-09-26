#include "bridge.h"

#include "print.h"

#include <stdlib.h>
#include <string.h>

namespace adk {

    namespace {

        constexpr Millis  Pace    = 100;        // at most ten messages a second
        constexpr Millis  Refresh = 2000;       // everything, again
        constexpr Millis  Silence = 5000;       // after which the other board is gone
        constexpr uint8_t Longest = 56;         // the shortest radio line, LoraLink's
        constexpr char    Mark    = '@';        // a bridge message, not stray text
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
        for (uint8_t index = 0; index < mineCount_; ++index)
        {
            Mine& mine = mine_[index];

            if (strcmp (mine.name, name) == 0)
            {
                mine.unsent = mine.unsent || mine.value != value;
                mine.value  = value;
                return;
            }
        }

        if (mineCount_ < MaxValues && strlen (name) <= NameLength)
        {
            mine_[mineCount_++] = {name, value, true};
        }
    }

    long Bridge::value (const char* name) const
    {
        const Theirs* theirs = find (name);
        return theirs ? theirs->value : 0;
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

    // "angle=90 speed=3": each name, an equals sign and a whole number. A
    // malformed pair ends the reading, as noise can't get past the radio's
    // checksum, so it can only come from something that isn't a bridge.
    void Bridge::hear (const char* text)
    {
        while (*text != '\0')
        {
            while (*text == ' ')
            {
                ++text;
            }

            const char* equals = strchr (text, '=');
            char*       end    = nullptr;

            if (!equals)
            {
                return;
            }

            size_t length = static_cast<size_t> (equals - text);
            long   number = strtol (equals + 1, &end, 10);

            if (length == 0 || length > NameLength || end == equals + 1)
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
                theirs->fresh = theirs->fresh || theirs->value != number;
                theirs->value = number;
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
            Text<24>    pair;

            print (pair, mine.name, '=', mine.value);

            if (!mine.unsent || line.size () + 1 + pair.size () > Longest)
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
