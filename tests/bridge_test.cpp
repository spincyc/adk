#include "check.h"

#include <Arduino.h>
#include <climits>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace {

    // One end of a perfect radio: what it sends lands in the other end's
    // queue, and each update hands on one line. It can be made busy, and
    // it remembers everything it sent.
    struct FakeRadio : adk::Object, adk::Link
    {
        bool sendLine (const char* text) override
        {
            if (busy)
            {
                return false;
            }

            sent.push_back (text);

            if (other)
            {
                other->queue.push_back (text);
            }

            return true;
        }

        const char* heardLine () const override
        {
            return heard ? line.c_str () : nullptr;
        }

        void update (adk::Millis) override
        {
            heard = !queue.empty ();

            if (heard)
            {
                line = queue.front ();
                queue.pop_front ();
            }
        }

        FakeRadio*               other = nullptr;
        std::deque<std::string>  queue;
        std::vector<std::string> sent;
        std::string              line;
        bool                     heard = false;
        bool                     busy  = false;
    };

    // Two boards' radios and bridges, updated together every 10 ms. Either
    // board can restart: its bridge starts again from nothing, and what
    // its serial port held is lost.
    struct Boards
    {
        Boards ()
        {
            radioA.other = &radioB;
            radioB.other = &radioA;
            bridgeA.emplace (radioA);
            bridgeB.emplace (radioB);
        }

        // Count the updates in which B heard a new "name".
        int run (adk::Millis ms, const char* name = "angle")
        {
            int changes = 0;

            for (adk::Millis step = 0; step < ms; step += 10)
            {
                now += 10;
                adk::update (now);
                changes += bridgeB->changed (name) ? 1 : 0;
            }

            return changes;
        }

        // Long enough for both boards to hear each other's start numbers.
        void meet ()
        {
            run (500);
            radioA.sent.clear ();
            radioB.sent.clear ();
        }

        void restartA ()
        {
            bridgeA.reset ();
            radioA.queue.clear ();
            bridgeA.emplace (radioA);
        }

        void restartB ()
        {
            bridgeB.reset ();
            radioB.queue.clear ();
            bridgeB.emplace (radioB);
        }

        FakeRadio                  radioA;
        FakeRadio                  radioB;
        std::optional<adk::Bridge> bridgeA;
        std::optional<adk::Bridge> bridgeB;
        adk::Millis                now = 0;
    };
}

TEST (bridgeCarriesAValueToTheOtherBoard)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->share ("angle", 90);
    CHECK (boards.run (200) == 1);
    CHECK (boards.bridgeB->value ("angle") == 90);
    CHECK (boards.radioA.sent.front () == "@0/0 angle=90");
    CHECK (boards.bridgeB->value ("nothing") == 0);
    CHECK (!boards.bridgeB->changed ("nothing"));

    // Once each has heard the other, both carry start number 1.
    boards.run (300);
    CHECK (boards.radioA.sent.back () == "@1/1 angle=90");
    boards.bridgeA->share ("angle", -45);
    CHECK (boards.run (200) == 1);
    CHECK (boards.bridgeB->value ("angle") == -45);
    CHECK (boards.radioA.sent.back () == "@1/1 angle=-45");
}

TEST (bridgeSendsAtMostTenMessagesASecondAndTheLatestValue)
{
    Boards boards;

    adk::setup ();

    // A knob turned fast: a new value on every update for a second.
    for (long angle = 0; angle < 100; ++angle)
    {
        boards.bridgeA->share ("angle", angle);
        boards.run (10);
    }

    boards.run (200);
    CHECK (boards.radioA.sent.size () <= 11);
    CHECK (boards.bridgeB->value ("angle") == 99);
}

TEST (bridgeSendsOnlyChangesButEverythingEveryTwoSeconds)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->share ("angle", 10);
    boards.bridgeA->share ("speed", 3);
    boards.meet ();
    CHECK (boards.bridgeB->value ("angle") == 10);
    CHECK (boards.bridgeB->value ("speed") == 3);

    boards.bridgeA->share ("angle", 10);
    boards.bridgeA->share ("speed", 4);
    boards.run (200);
    CHECK (boards.radioA.sent.size () == 1);
    CHECK (boards.radioA.sent.back () == "@1/1 speed=4");

    // The refresh sends both again, which isn't a change on B.
    CHECK (boards.run (2000, "speed") == 0);
    CHECK (boards.radioA.sent.back () == "@1/1 angle=10 speed=4");
}

TEST (bridgeKnowsWhenTheOtherBoardIsThere)
{
    Boards boards;

    adk::setup ();
    CHECK (!boards.bridgeA->isConnected ());

    // B shares nothing, but says it's there as it starts, and every two
    // seconds after that.
    boards.bridgeA->share ("angle", 1);
    boards.run (2500);
    CHECK (boards.bridgeA->isConnected ());
    CHECK (boards.bridgeB->isConnected ());
    CHECK (boards.radioB.sent.front () == "@0/0");
    CHECK (boards.radioB.sent.back () == "@1/1");

    boards.radioB.other = nullptr;
    boards.run (4000);
    CHECK (boards.bridgeA->isConnected ());
    boards.run (2000);
    CHECK (!boards.bridgeA->isConnected ());
}

TEST (bridgeTakesAStartNumberOneMoreThanTheOtherRemembers)
{
    Boards boards;

    adk::setup ();
    boards.radioA.other = nullptr;
    boards.radioB.other = nullptr;

    boards.radioB.queue.push_back ("@3/7");
    boards.run (110);
    CHECK (boards.radioB.sent.back () == "@8/3");

    // They go round from 99 to 1, never 0, which means not started yet.
    boards.radioA.queue.push_back ("@5/99");
    boards.run (110);
    CHECK (boards.radioA.sent.back () == "@1/5");
}

TEST (bridgeSplitsManyValuesAcrossMessages)
{
    Boards boards;

    adk::setup ();
    const char* names [] = {"alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf",
                            "hotel"};

    for (const char* name : names)
    {
        boards.bridgeA->share (name, -2000000000L);
    }

    boards.run (1000);
    CHECK (boards.radioA.sent.size () > 1);

    for (const std::string& line : boards.radioA.sent)
    {
        CHECK (line.size () <= 56);
    }

    for (const char* name : names)
    {
        CHECK (boards.bridgeB->value (name) == -2000000000L);
    }

    // A ninth name, or one too long, is left out.
    boards.bridgeA->share ("india", 9);
    boards.bridgeA->share ("toolongname", 9);
    boards.run (3000);
    CHECK (boards.bridgeB->value ("india") == 0);
}

TEST (bridgeTriesAgainWhenTheRadioIsBusy)
{
    Boards boards;

    adk::setup ();
    boards.radioA.busy = true;
    boards.bridgeA->share ("angle", 5);
    boards.run (500);
    CHECK (boards.bridgeB->value ("angle") == 0);

    boards.radioA.busy = false;
    boards.run (200);
    CHECK (boards.bridgeB->value ("angle") == 5);
}

TEST (bridgeIgnoresTextThatIsNotABridgeMessage)
{
    Boards boards;

    adk::setup ();
    boards.radioA.other = nullptr;
    boards.radioB.queue = {"hello there", "@1/x angle=1", "@1/2/3 angle=2", "@300/1 angle=3",
                           "@1/ angle=4", "@/1 angle=5"};
    boards.run (100);
    CHECK (boards.bridgeB->value ("angle") == 0);
    CHECK (!boards.bridgeB->isConnected ());

    // A line typed by hand, without start numbers, still counts.
    boards.radioB.queue = {"@angle", "@=5", "@angle=", "@angle=7 speed=x"};
    boards.run (100);
    CHECK (boards.bridgeB->value ("angle") == 7);
    CHECK (boards.bridgeB->value ("speed") == 0);
}

TEST (bridgeCarriesEachNewEventOnceWithItsPayload)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->shareEvent ("key", 0, 0);
    boards.meet ();
    CHECK (boards.bridgeB->payload ("key") == 0);
    CHECK (boards.radioA.sent.empty ());

    // changed () lasts the one update the event arrives in, however often
    // it is read.
    boards.bridgeA->shareEvent ("key", 1, 7);

    for (int step = 0; step < 20 && !boards.bridgeB->changed ("key"); ++step)
    {
        boards.run (10);
    }

    CHECK (boards.bridgeB->changed ("key"));
    CHECK (boards.bridgeB->changed ("key"));
    CHECK (boards.bridgeB->value ("key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 7);
    CHECK (boards.radioA.sent.back () == "@1/1 key=1:7");
    boards.run (10);
    CHECK (!boards.bridgeB->changed ("key"));

    // The same key again is a new event, because its count went up.
    boards.bridgeA->shareEvent ("key", 2, 7);
    CHECK (boards.run (100, "key") == 1);
    CHECK (boards.bridgeB->value ("key") == 2);
    CHECK (boards.bridgeB->payload ("key") == 7);
    CHECK (boards.radioA.sent.back () == "@1/1 key=2:7");

    // Sharing it again, or the refresh, is no new event.
    boards.bridgeA->shareEvent ("key", 2, 7);
    CHECK (boards.run (2100, "key") == 0);
    CHECK (boards.radioA.sent.back () == "@1/1 key=2:7");
}

TEST (bridgeEventKeepsItsOwnPayloadAfterALostMessage)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->shareEvent ("key", 0, 0);
    boards.meet ();
    boards.bridgeA->shareEvent ("key", 1, 7);
    boards.run (200);

    // Lose the event that brought the new payload, then repeat that
    // payload as the next event: B never pairs 9 with an old count.
    boards.radioA.other = nullptr;
    boards.bridgeA->shareEvent ("key", 2, 9);
    CHECK (boards.run (200, "key") == 0);
    CHECK (boards.bridgeB->value ("key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 7);

    boards.radioA.other = &boards.radioB;
    boards.bridgeA->shareEvent ("key", 3, 9);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.radioA.sent.back () == "@1/1 key=3:9");
    CHECK (boards.bridgeB->value ("key") == 3);
    CHECK (boards.bridgeB->payload ("key") == 9);
}

TEST (bridgeEventsWhileTheRadioIsBusyArriveAsTheLatest)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->shareEvent ("key", 0, 0);
    boards.meet ();
    boards.radioA.busy = true;
    boards.bridgeA->shareEvent ("key", 1, 7);
    boards.run (200);
    boards.bridgeA->shareEvent ("key", 2, 9);
    boards.run (200);
    CHECK (boards.radioA.sent.empty ());
    CHECK (!boards.bridgeB->changed ("key"));

    boards.radioA.busy = false;
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.radioA.sent.size () == 1);
    CHECK (boards.radioA.sent.back () == "@1/1 key=2:9");
    CHECK (boards.bridgeB->value ("key") == 2);
    CHECK (boards.bridgeB->payload ("key") == 9);
}

TEST (bridgeEventsFromBeforeTheBoardsMetNeverArrive)
{
    Boards boards;

    adk::setup ();

    // A counts three events alone, and is heard only once it has started.
    boards.radioA.other = nullptr;
    boards.radioB.other = nullptr;
    boards.bridgeA->shareEvent ("key", 3, 5);
    boards.bridgeA->share ("angle", 40);
    boards.run (300);
    boards.radioA.other = &boards.radioB;
    boards.radioB.other = &boards.radioA;
    CHECK (boards.run (2500, "key") == 0);
    CHECK (boards.bridgeB->value ("angle") == 40);
    CHECK (boards.bridgeB->isConnected ());

    boards.bridgeA->shareEvent ("key", 4, 6);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.bridgeB->value ("key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 6);
}

TEST (bridgeRecoversWhenOneBoardRestarts)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->share ("angle", 30);
    boards.bridgeA->shareEvent ("key", 5, 1);
    boards.bridgeB->share ("typed", 2);
    boards.meet ();
    boards.bridgeA->shareEvent ("key", 6, 2);
    CHECK (boards.run (200, "key") == 1);

    // B restarts. A carries on, and nobody resets it: within half a second
    // B has A's values again, long before the two-second refresh, and A's
    // last event is not mistaken for a new one.
    boards.restartB ();
    CHECK (boards.run (500, "key") == 0);
    CHECK (boards.bridgeB->value ("angle") == 30);
    CHECK (boards.bridgeB->isConnected ());
    CHECK (boards.bridgeA->isConnected ());
    CHECK (boards.bridgeA->value ("typed") == 2);    // as B last said, before it restarted

    // A's next event is the first B hears of, counted from when they met.
    boards.bridgeA->shareEvent ("key", 7, 3);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.bridgeB->value ("key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 3);

    // Now A restarts, its own count back at 0. B carries on, and A's first
    // new event still arrives.
    boards.restartA ();
    boards.bridgeA->shareEvent ("key", 0, 0);
    boards.run (500, "key");
    CHECK (boards.bridgeA->isConnected ());
    CHECK (boards.bridgeB->isConnected ());
    boards.bridgeA->shareEvent ("key", 1, 4);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 4);
}

TEST (adkStopSilencesEveryBridgeUntilItStartsAgain)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->share ("angle", 30);
    boards.bridgeA->shareEvent ("key", 0, 0);
    boards.meet ();

    // Stopped, A sends nothing, not even the two-second refresh, and B
    // soon takes it for gone.
    adk::stop ();
    boards.bridgeA->share ("angle", 40);
    boards.bridgeA->shareEvent ("key", 1, 7);
    CHECK (boards.run (6000, "angle") == 0);
    CHECK (boards.radioA.sent.empty ());
    CHECK (boards.bridgeB->value ("angle") == 30);
    CHECK (!boards.bridgeB->isConnected ());

    // B, stopped too, still hears A once A starts again, so its values keep
    // up; but nothing is news to it, so a loop driven by changed () stays
    // still.
    boards.bridgeA->start ();
    CHECK (boards.run (200, "angle") == 0);
    CHECK (boards.radioA.sent.front () == "@1/1 angle=40 key=1:7");
    CHECK (boards.bridgeB->value ("angle") == 40);
    CHECK (boards.bridgeB->value ("key") == 1);
    CHECK (!boards.bridgeB->changed ("key"));
    CHECK (boards.bridgeB->isConnected ());
    CHECK (boards.radioB.sent.empty ());

    // Started again, B tells A everything at once. What changed while it
    // was stopped is no news; the next change is.
    boards.bridgeB->start ();
    CHECK (boards.run (200, "key") == 0);
    CHECK (boards.radioB.sent.size () == 1);
    CHECK (boards.bridgeA->isConnected ());
    boards.bridgeA->shareEvent ("key", 2, 8);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 8);
    boards.bridgeA->share ("angle", 41);
    CHECK (boards.run (200, "angle") == 1);
}

TEST (aBridgeStoppedMidPassStillReportsWhatArrived)
{
    Boards boards;

    adk::setup ();
    boards.meet ();
    boards.bridgeA->share ("angle", 30);

    while (!boards.bridgeB->changed ("angle"))
    {
        boards.run (10);
    }

    // Nothing a sketch does takes an event back before the next update.
    boards.bridgeB->stop ();
    CHECK (boards.bridgeB->changed ("angle"));
    boards.run (10);
    CHECK (!boards.bridgeB->changed ("angle"));
}

TEST (bridgeIgnoresAReplyMeantForTheBoardBeforeItRestarted)
{
    Boards boards;

    adk::setup ();
    boards.bridgeB->share ("typed", 3);
    boards.bridgeB->shareEvent ("ack", 4, 9);
    boards.meet ();
    CHECK (boards.bridgeA->value ("typed") == 3);

    // A restarts, and the first thing it hears is a message B sent to A as
    // it was: start numbers 1/1, but A is no longer 1.
    boards.restartA ();
    boards.radioA.queue.push_back ("@1/1 typed=3 ack=5:9");
    adk::update (boards.now += 10);
    CHECK (boards.bridgeA->value ("typed") == 0);
    CHECK (!boards.bridgeA->changed ("typed"));
    CHECK (!boards.bridgeA->changed ("ack"));
    CHECK (!boards.bridgeA->isConnected ());

    // B hears A's new start number and tells A everything again at once.
    boards.run (400);
    CHECK (boards.bridgeA->value ("typed") == 3);
    CHECK (boards.bridgeA->value ("ack") == 0);
    CHECK (boards.bridgeA->isConnected ());
    CHECK (boards.radioA.sent.back () == "@2/1");
    CHECK (boards.radioB.sent.back () == "@1/2 typed=3 ack=0:9");
}

TEST (bridgeKeepsTimeAcrossTheWrapOfMillis)
{
    Boards boards;

    // Start three seconds before millis () goes round to 0.
    boards.now = 0xFFFFFFFFUL - 3000;
    adk::setup ();
    boards.bridgeA->share ("angle", 0);
    boards.meet ();

    // At most ten messages a second, right across the wrap.
    for (long angle = 1; angle <= 300; ++angle)
    {
        boards.bridgeA->share ("angle", angle);
        boards.run (10);
    }

    CHECK (boards.radioA.sent.size () <= 31);
    CHECK (boards.radioA.sent.size () >= 29);

    // Then nothing but the refresh, every two seconds.
    boards.run (100);
    CHECK (boards.bridgeB->value ("angle") == 300);
    boards.radioA.sent.clear ();
    boards.run (6000);
    CHECK (boards.radioA.sent.size () == 3);

    // Five seconds after B falls silent, A knows it has gone, and still
    // knows it when millis () comes all the way round to the time it last
    // heard B, 49.7 days later.
    boards.bridgeB->share ("seen", 1);
    boards.run (200);
    boards.radioB.other = nullptr;
    boards.run (4700);
    CHECK (boards.bridgeA->isConnected ());

    for (int step = 0; step < 100 && boards.bridgeA->isConnected (); ++step)
    {
        boards.run (10);
    }

    CHECK (!boards.bridgeA->isConnected ());
    boards.now += 0U - 5000U;
    adk::update (boards.now);
    CHECK (!boards.bridgeA->isConnected ());
}

TEST (bridgeSplitsFullSizeEventsWithoutSplittingTheirPairs)
{
    Boards boards;

    adk::setup ();
    constexpr long Lowest = -2147483647L - 1;
    const char* names [] = {"alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf",
                            "hotel"};

    boards.bridgeA->shareEvent ("toolong8", 0, 2);

    for (const char* name : names)
    {
        boards.bridgeA->shareEvent (name, 0, 0);
    }

    boards.meet ();

    for (const char* name : names)
    {
        boards.bridgeA->shareEvent (name, -2147483647L, Lowest);
    }

    boards.run (1000);
    CHECK (boards.radioA.sent.size () == 8);

    for (const std::string& line : boards.radioA.sent)
    {
        CHECK (line.size () <= 56);
        CHECK (line.find ("=-2147483647:-2147483648") != std::string::npos);
    }

    // The count went from 0 to -2147483647: round the 32 bits, as the
    // Mega's long does, that is down, so no new event.
    for (const char* name : names)
    {
        CHECK (boards.bridgeB->value (name) == -2147483647L);
        CHECK (boards.bridgeB->payload (name) == Lowest);
        CHECK (!boards.bridgeB->changed (name));
    }

    CHECK (boards.bridgeB->value ("toolong8") == 0);
    boards.bridgeA->share ("india", 9);
    boards.bridgeA->shareEvent ("juliet", 10, 11);
    boards.run (3000);
    CHECK (boards.bridgeB->value ("india") == 0);
    CHECK (boards.bridgeB->value ("juliet") == 0);
}

TEST (bridgeEventsPreserveSigned32BitPayloadsAndUidBits)
{
    Boards boards;

    adk::setup ();
    constexpr long Lowest  = -2147483647L - 1;
    constexpr long Highest = 2147483647L;

    boards.bridgeA->shareEvent ("low", 0, 0);
    boards.bridgeA->shareEvent ("high", 0, 0);
    boards.bridgeA->shareEvent ("uid", 0, 0);
    boards.meet ();
    boards.bridgeA->shareEvent ("low", 1, Lowest);
    boards.bridgeA->shareEvent ("high", 1, Highest);
    boards.bridgeA->shareEvent ("uid", 1, static_cast<int32_t> (0xfedcba98UL));
    boards.run (500);
    CHECK (boards.bridgeB->payload ("low") == Lowest);
    CHECK (boards.bridgeB->payload ("high") == Highest);
    CHECK (static_cast<uint32_t> (boards.bridgeB->payload ("uid")) == 0xfedcba98UL);

    if constexpr (LONG_MAX > Highest)
    {
        // A host's wider long must not produce a token the Mega can't read.
        boards.bridgeA->shareEvent ("low", LONG_MAX, 1);
        boards.bridgeA->shareEvent ("low", 2, LONG_MIN);
        CHECK (boards.run (200, "low") == 0);
        CHECK (boards.bridgeB->value ("low") == 1);
        CHECK (boards.bridgeB->payload ("low") == Lowest);
    }
}

TEST (bridgeRejectsMalformedEventsBeforeChangingEitherNumber)
{
    Boards boards;

    adk::setup ();
    boards.radioA.other = nullptr;    // only these lines, typed by hand
    boards.radioB.queue.push_back ("@key=7:9");
    CHECK (boards.run (10, "key") == 1);

    const char* malformed [] = {
        "@key=:4", "@key=8:", "@key=8:no", "@key=8:+", "@key=8:-",
        "@key=8: 4", "@key=8::4", "@key=8:4:5", "@key=8:4x", "@key=8x",
        "@key=8:2147483648", "@key=8:-2147483649", "@key=2147483648:4",
        "@key=-2147483649:4", "@key=99999999999999999999999:4",
        "@key=8:99999999999999999999999", "@key=8: other=4", "@key= 8:4"
    };

    for (const char* token : malformed)
    {
        boards.radioB.queue.push_back (token);
        CHECK (boards.run (10, "key") == 0);
        CHECK (boards.bridgeB->value ("key") == 7);
        CHECK (boards.bridgeB->payload ("key") == 9);
        CHECK (boards.bridgeB->value ("other") == 0);
    }
}

TEST (bridgeMalformedEventsDoNotTakeSlots)
{
    Boards boards;

    adk::setup ();
    boards.radioA.other = nullptr;    // only these lines, typed by hand
    boards.radioB.queue = {"@one=1:", "@two=1:", "@three=1:", "@four=1:",
                           "@five=1:", "@six=1:", "@seven=1:", "@eight=1:",
                           "@toolong8=1:2", "@=1:2", "@valid=1:0"};
    CHECK (boards.run (110, "valid") == 1);
    CHECK (boards.bridgeB->changed ("valid"));
    CHECK (boards.bridgeB->value ("valid") == 1);
    CHECK (boards.bridgeB->payload ("valid") == 0);
}

TEST (bridgeEventChangesSurviveDuplicatesAndLaterMalformedTokens)
{
    Boards boards;

    adk::setup ();
    boards.radioA.other = nullptr;    // only these lines, typed by hand
    boards.radioB.queue.push_back ("@key=2:7 key=3:8 key=3:8");
    CHECK (boards.run (10, "key") == 1);
    CHECK (boards.bridgeB->value ("key") == 3);
    CHECK (boards.bridgeB->payload ("key") == 8);

    boards.radioB.queue.push_back ("@key=4:9 key=5:");
    CHECK (boards.run (10, "key") == 1);
    CHECK (boards.bridgeB->value ("key") == 4);
    CHECK (boards.bridgeB->payload ("key") == 9);
    CHECK (boards.run (10, "key") == 0);
}

TEST (bridgeCanReplaceAScalarWithAnEventAndBack)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA->share ("key", 1);
    CHECK (boards.run (500, "key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 0);

    // The count A had as a value is where its events start.
    boards.bridgeA->shareEvent ("key", 1, 0);
    CHECK (boards.run (200, "key") == 0);
    CHECK (boards.radioA.sent.back () == "@1/1 key=0:0");

    boards.bridgeA->shareEvent ("key", 2, 7);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 7);

    boards.bridgeA->share ("key", 1);
    CHECK (boards.run (200, "key") == 1);
    CHECK (boards.bridgeB->value ("key") == 1);
    CHECK (boards.bridgeB->payload ("key") == 0);
    CHECK (boards.radioA.sent.back () == "@1/1 key=1");
}

TEST (loraModemSendsToItsPartnerWithItsSettings)
{
    std::vector<std::string> commands;
    std::string              line;

    Serial3.onWrite = [&] (uint8_t byte)
    {
        line += static_cast<char> (byte);

        if (byte == '\n')
        {
            commands.push_back (line.substr (0, line.size () - 2));
            line.clear ();
            Serial3.input += "+OK\r\n";
        }
    };

    adk::LoraModem modem  {Serial3, 1, {.partner = 2, .speed = adk::LoraSpeed::Quick,
                                        .power = 10}};
    adk::Bridge    bridge {modem};

    adk::setup ();
    CHECK (modem.ok ());
    CHECK (commands.size () == 6);
    CHECK (commands[4] == "AT+CRFOP=10");
    CHECK (commands[5] == "AT+PARAMETER=7,7,1,4");

    bridge.share ("angle", 30);
    adk::update (100);
    CHECK (commands.back () == "AT+SEND=2,13,@0/0 angle=30");

    Serial3.input += "+RCV=2,14,@1/0 light=512,-40,9\r\n";
    adk::update (200);
    CHECK (bridge.changed ("light"));
    CHECK (bridge.value ("light") == 512);
    CHECK (bridge.isConnected ());

    adk::update (300);
    CHECK (commands.back () == "AT+SEND=2,13,@1/1 angle=30");
}
