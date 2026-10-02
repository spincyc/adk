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
    // A message is plain text, "@1/2 angle=90 speed=3", so it can be read in
    // the Serial Monitor, and names are short words in quotes, up to 7
    // letters. The two numbers after the @ are start numbers: this board's,
    // then the other board's as this board last heard it. A board takes a
    // new start number each time it starts, one more than the other board
    // remembers, so when either board restarts, both notice. The other
    // board sends everything again at once, and a message meant for the
    // board as it was before it restarted is ignored.
    struct Bridge : Object
    {
        // Over one radio both ways, or a transmitter out and a receiver in.
        explicit Bridge (Link& radio);
        Bridge          (Link& out, Link& in);

        // Tell the other board a value. Call it as often as you like: only a
        // change is sent. Up to eight names, shared with shareEvent ().
        void share (const char* name, long value);

        // Tell the other board about events, such as key presses: count is
        // how many there have been, and payload goes with the latest, such
        // as which key. Add one to count for every event, even when its
        // payload is the same as the last, and call it as often as you
        // like. Only the latest event is kept, so two in one tenth of a
        // second arrive as one.
        void shareEvent (const char* name, long count, long payload);

        // The other board's latest value by that name, or 0 until one has
        // arrived. changed () lasts one update: for a value, the update in
        // which it changes or first arrives; for an event, the update in
        // which a new one arrives, and payload () is what came with it. An
        // event from before the two boards last found each other, at the
        // start or after a restart, never arrives. An event's value () is
        // its count since then.
        long value   (const char* name) const;
        long payload (const char* name) const;
        bool changed (const char* name) const;

        // Heard from the other board in the last five seconds.
        bool isConnected () const;

        // Stop sending: nothing goes out, not even the two-second refresh,
        // until start (). A stopped bridge still listens, so value () keeps
        // up, but no value or event counts as changed (). adk::stop () stops
        // every Bridge too, so a loop driven by changed () stays still.
        void stop () override;

        // Talk again after stop (), sending everything at once. What changed
        // while it was stopped is not news: changed () waits for the next
        // change.
        void start ();

        static constexpr uint8_t MaxValues  = 8;
        static constexpr uint8_t NameLength = 7;

      protected:
        void update (Millis now) override;

      private:
        // An event's value is the sketch's own count, and start what it was
        // when the boards last found each other: the air carries the
        // difference, so the other board's tally starts again from 0.
        struct Mine
        {
            const char* name;
            long        value;
            long        payload;
            long        start;
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

        void          keep    (const char* name, long value, long payload, bool event);
        void          hear    (const char* text);
        void          meet    (uint8_t run, uint8_t echo);
        void          restart ();
        void          tell    (Millis now);
        const Theirs* find    (const char* name) const;

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
        uint8_t next_;
        uint8_t run_;          // this board's start number, 0 until it hears the other
        uint8_t theirRun_;     // the other board's, as last heard
        bool    beat_;
        bool    heard_;
        bool    stopped_;
    };
}
