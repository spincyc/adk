#pragma once

#include "link.h"
#include "object.h"

namespace adk {

    // Keeps a few named numbers the same on two boards, over a radio. One
    // board shares a value, say bridge.share ("angle", 90); the other reads
    // bridge.value ("angle"), and bridge.changed ("angle") is true in the
    // update a new one arrives.
    //
    // A change goes out at once, but at most ten messages a second, so a knob
    // turned fast doesn't flood the air: the other board gets its latest
    // value. Every two seconds everything is sent again, so a message lost
    // to noise is soon made good, and each board knows whether the other is
    // still there. Declare the bridge after its radio.
    //
    // A message is plain text, "@angle=90 speed=3", so it can be read in the
    // Serial Monitor, and names are short words in quotes, up to 7 letters.
    struct Bridge : Object
    {
        // Over one radio both ways, or a transmitter out and a receiver in.
        explicit Bridge (Link& radio);
        Bridge          (Link& out, Link& in);

        // Tell the other board a value. Call it as often as you like: only a
        // change is sent. Up to eight names.
        void share (const char* name, long value);

        // The other board's latest value by that name, or 0 until one has
        // arrived; and whether a new one arrived in this update.
        long value   (const char* name) const;
        bool changed (const char* name) const;

        // Heard from the other board in the last five seconds.
        bool isConnected () const;

        static constexpr uint8_t MaxValues  = 8;
        static constexpr uint8_t NameLength = 7;

      protected:
        void update (Millis now) override;

      private:
        struct Mine
        {
            const char* name;
            long        value;
            bool        unsent;
        };

        struct Theirs
        {
            char name [NameLength + 1];
            long value;
            bool fresh;
        };

        void          hear (const char* text);
        void          tell (Millis now);
        const Theirs* find (const char* name) const;

        Link&   out_;
        Link&   in_;
        Mine    mine_   [MaxValues];
        Theirs  theirs_ [MaxValues];
        Millis  now_;
        Millis  sentAt_;
        Millis  refreshedAt_;
        Millis  heardAt_;
        uint8_t mineCount_;
        uint8_t theirsCount_;
        bool    beat_;
        bool    heard_;
    };
}
