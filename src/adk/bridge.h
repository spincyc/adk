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
        // change is sent. Up to eight names, shared with shareEvent ().
        void share (const char* name, long value);

        // Send an event's sequence and payload together. Change the sequence
        // for each event, even when its payload is the same. Only the latest
        // pair is kept; this is not a queue or a delivery acknowledgement.
        // Both numbers use the Mega's signed 32-bit long range.
        void shareEvent (const char* name, long sequence, long payload);

        // The other board's latest value by that name, or 0 until one has
        // arrived. For an event, value () is its sequence and payload () is
        // its payload; a scalar's payload is 0. changed () lasts one update
        // when either number or its kind changes, or the name first arrives.
        long value   (const char* name) const;
        long payload (const char* name) const;
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
            long        payload;
            bool        event;
            bool        unsent;
        };

        struct Theirs
        {
            char name [NameLength + 1];
            long value;
            long payload;
            bool event;
            bool fresh;
        };

        void          keep (const char* name, long value, long payload, bool event);
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
