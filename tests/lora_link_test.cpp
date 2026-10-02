#include "check.h"

#include <Arduino.h>
#include <string>

namespace {

    constexpr adk::Pin Mode = 40;
    constexpr adk::Pin Aux  = 41;

    void configure (adk::LoraLink& link)
    {
        Serial3.onWrite = [] (uint8_t)
        {
            if (Serial3.text.size () == 6)
            {
                Serial3.input = Serial3.text;
            }
        };

        adk::setup ();
        CHECK (link.ok ());
        Serial3.text.clear ();
        Serial3.onWrite = nullptr;
    }
}

TEST (loraLinkRejectsABusyModuleWithoutWaitingOrWriting)
{
    adk::LoraLink link {Serial3, Mode, Aux};
    configure (link);

    arduino::pin (Aux).input = LOW;
    unsigned long before = micros ();
    CHECK (!link.send ("should wait"));
    CHECK (Serial3.text.empty ());
    CHECK (micros () == before);

    arduino::pin (Aux).input = HIGH;
    CHECK (link.send ("retry"));
    CHECK (Serial3.text == "retry\n");
}

TEST (loraLinkReservesTheUartUntilTheModuleCanAssertAux)
{
    adk::LoraLink link {Serial3, Mode, Aux};
    configure (link);
    std::string longest (adk::LoraLink::MaxLength, 'x');

    CHECK (link.send (longest.c_str ()));
    CHECK (!link.send ("too soon"));
    CHECK (Serial3.text == longest + '\n');

    // At 9600 8N1, 57 bytes plus the E32's three-byte idle detector
    // need 62.5 ms. The first update starts the reservation clock.
    adk::update (100);
    adk::update (162);
    CHECK (!link.send ("still too soon"));
    adk::update (163);
    arduino::pin (Aux).input = LOW;
    CHECK (!link.send ("module is still busy"));
    CHECK (Serial3.text == longest + '\n');

    arduino::pin (Aux).input = HIGH;
    CHECK (link.send ("next"));
    CHECK (!link.send ("same pass"));
    CHECK (Serial3.text == longest + "\nnext\n");
}

TEST (loraLinkReservationCrossesTheClockWrapEvenIfAuxWasNotSampledLow)
{
    adk::LoraLink link {Serial3, Mode, Aux};
    configure (link);

    CHECK (link.send ("x"));
    adk::update (0xFFFFFFFEUL);
    adk::update (3);
    CHECK (!link.send ("early"));

    // Two UART bytes and three idle byte-times need ceil (5.208) ms.
    adk::update (4);
    CHECK (link.send ("later"));
    CHECK (Serial3.text == "x\nlater\n");
}

TEST (loraLinkLeavesBridgeValuesPendingWhileAuxIsLow)
{
    adk::LoraLink link   {Serial3, Mode, Aux};
    adk::Bridge   bridge {link};
    configure (link);

    arduino::pin (Aux).input = LOW;
    bridge.share ("angle", 1);
    adk::update (100);
    CHECK (Serial3.text.empty ());
    bridge.share ("angle", 2);
    adk::update (110);
    CHECK (Serial3.text.empty ());

    arduino::pin (Aux).input = HIGH;
    adk::update (120);
    CHECK (Serial3.text == "@0/0 angle=2\n");
}
