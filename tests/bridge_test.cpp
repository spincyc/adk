#include "check.h"

#include <Arduino.h>
#include <deque>
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

    // Two boards' radios and bridges, updated together every 10 ms.
    struct Boards
    {
        Boards ()
        {
            radioA.other = &radioB;
            radioB.other = &radioA;
        }

        // Count the updates in which B heard a new "name".
        int run (adk::Millis ms, const char* name = "angle")
        {
            int changes = 0;

            for (adk::Millis step = 0; step < ms; step += 10)
            {
                now += 10;
                adk::update (now);
                changes += bridgeB.changed (name) ? 1 : 0;
            }

            return changes;
        }

        FakeRadio   radioA;
        FakeRadio   radioB;
        adk::Bridge bridgeA {radioA};
        adk::Bridge bridgeB {radioB};
        adk::Millis now = 0;
    };
}

TEST (bridgeCarriesAValueToTheOtherBoard)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA.share ("angle", 90);
    CHECK (boards.run (200) == 1);
    CHECK (boards.bridgeB.value ("angle") == 90);
    CHECK (boards.radioA.sent.front () == "@angle=90");
    CHECK (boards.bridgeB.value ("nothing") == 0);
    CHECK (!boards.bridgeB.changed ("nothing"));

    boards.bridgeA.share ("angle", -45);
    CHECK (boards.run (200) == 1);
    CHECK (boards.bridgeB.value ("angle") == -45);
}

TEST (bridgeSendsAtMostTenMessagesASecondAndTheLatestValue)
{
    Boards boards;

    adk::setup ();

    // A knob turned fast: a new value on every update for a second.
    for (long angle = 0; angle < 100; ++angle)
    {
        boards.bridgeA.share ("angle", angle);
        boards.run (10);
    }

    boards.run (200);
    CHECK (boards.radioA.sent.size () <= 11);
    CHECK (boards.bridgeB.value ("angle") == 99);
}

TEST (bridgeSendsOnlyChangesButEverythingEveryTwoSeconds)
{
    Boards boards;

    adk::setup ();
    boards.bridgeA.share ("angle", 10);
    boards.bridgeA.share ("speed", 3);
    boards.run (200);
    CHECK (boards.radioA.sent.size () == 1);
    CHECK (boards.radioA.sent.back () == "@angle=10 speed=3");

    boards.bridgeA.share ("angle", 10);
    boards.bridgeA.share ("speed", 4);
    boards.run (200);
    CHECK (boards.radioA.sent.back () == "@speed=4");

    // The refresh sends both again, which isn't a change on B.
    CHECK (boards.run (2000, "speed") == 0);
    CHECK (boards.radioA.sent.back () == "@angle=10 speed=4");
}

TEST (bridgeKnowsWhenTheOtherBoardIsThere)
{
    Boards boards;

    adk::setup ();
    CHECK (!boards.bridgeA.isConnected ());

    // B shares nothing, but says it's there every two seconds.
    boards.bridgeA.share ("angle", 1);
    boards.run (2500);
    CHECK (boards.bridgeA.isConnected ());
    CHECK (boards.bridgeB.isConnected ());
    CHECK (boards.radioB.sent.front () == "@");

    boards.radioB.other = nullptr;
    boards.run (4000);
    CHECK (boards.bridgeA.isConnected ());
    boards.run (2000);
    CHECK (!boards.bridgeA.isConnected ());
}

TEST (bridgeSplitsManyValuesAcrossMessages)
{
    Boards boards;

    adk::setup ();
    const char* names [] = {"alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf",
                            "hotel"};

    for (const char* name : names)
    {
        boards.bridgeA.share (name, -2000000000L);
    }

    boards.run (1000);
    CHECK (boards.radioA.sent.size () > 1);

    for (const std::string& line : boards.radioA.sent)
    {
        CHECK (line.size () <= 56);
    }

    for (const char* name : names)
    {
        CHECK (boards.bridgeB.value (name) == -2000000000L);
    }

    // A ninth name, or one too long, is left out.
    boards.bridgeA.share ("india", 9);
    boards.bridgeA.share ("toolongname", 9);
    boards.run (3000);
    CHECK (boards.bridgeB.value ("india") == 0);
}

TEST (bridgeTriesAgainWhenTheRadioIsBusy)
{
    Boards boards;

    adk::setup ();
    boards.radioA.busy = true;
    boards.bridgeA.share ("angle", 5);
    boards.run (500);
    CHECK (boards.bridgeB.value ("angle") == 0);

    boards.radioA.busy = false;
    boards.run (200);
    CHECK (boards.bridgeB.value ("angle") == 5);
}

TEST (bridgeIgnoresTextThatIsNotABridgeMessage)
{
    Boards boards;

    adk::setup ();
    boards.radioB.queue = {"hello there", "@angle", "@=5", "@angle=", "@angle=7 speed=x"};
    boards.run (100);
    CHECK (boards.bridgeB.value ("angle") == 7);
    CHECK (boards.bridgeB.value ("speed") == 0);
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
    CHECK (commands.back () == "AT+SEND=2,9,@angle=30");

    Serial3.input += "+RCV=2,10,@light=512,-40,9\r\n";
    adk::update (200);
    CHECK (bridge.changed ("light"));
    CHECK (bridge.value ("light") == 512);
    CHECK (bridge.isConnected ());
}
