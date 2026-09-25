#include "check.h"

#include <Arduino.h>
#include <string>
#include <vector>

namespace {

    // Plays a REYAX modem: answers +OK to every command, and remembers them.
    struct Modem
    {
        explicit Modem (HardwareSerial& port, bool answers = true)
        {
            port.onWrite = [this, &port, answers] (uint8_t byte)
            {
                line += static_cast<char> (byte);

                if (byte != '\n')
                {
                    return;
                }

                commands.push_back (line.substr (0, line.size () - 2));
                line.clear ();

                if (answers)
                {
                    port.input += "+OK\r\n";
                }
            };
        }

        std::string              line;
        std::vector<std::string> commands;
    };

    // Plays an E32: after six bytes of settings, it sends them back.
    struct Module
    {
        Module (HardwareSerial& port, adk::Pin mode)
        {
            port.onWrite = [this, &port, mode] (uint8_t byte)
            {
                received += static_cast<char> (byte);
                inSettings.push_back (arduino::pin (mode).mode == INPUT);

                if (received.size () == 6 && echo)
                {
                    port.input += received;
                }
            };
        }

        std::string       received;
        std::vector<bool> inSettings;
        bool              echo = true;
    };
}

TEST (serialPortsClaimTheirPins)
{
    CHECK (adk::claimSerial (Serial1));
    CHECK (arduino::pin (18).mode == OUTPUT);
    CHECK (arduino::pin (18).output == HIGH);
    CHECK (arduino::pin (19).mode == INPUT);
    CHECK (adk::claimSerial (Serial2));
    CHECK (adk::isClaimed (16) && adk::isClaimed (17));
    CHECK (adk::claimSerial (Serial3));
    CHECK (adk::isClaimed (14) && adk::isClaimed (15));

    CHECK (!adk::claimSerial (Serial));
    CHECK (adk::fault () == adk::Fault::NotSerial);
    CHECK (adk::faultPin () == 1);

    arduino::Log log;
    adk::explain (log, adk::Fault::NotSerial, 1);
    CHECK (log.text.find ("Serial1, Serial2 or Serial3") != std::string::npos);
}

TEST (lineReaderGathersWholeLinesWithoutWaiting)
{
    adk::LineReader<8> reader;

    Serial2.input = "hel";
    CHECK (!reader.read (Serial2));
    Serial2.input = "lo\r\nnext";
    CHECK (reader.read (Serial2));
    CHECK (std::string (reader.line ()) == "hello");
    CHECK (!reader.read (Serial2));
    CHECK (std::string (reader.line ()) == "next");

    Serial2.input = " line that is far too long\n";
    CHECK (reader.read (Serial2));
    CHECK (std::string (reader.line ()) == "next lin");
    CHECK (reader.size () == 8);
}

TEST (loraModemIsSetUpEveryTime)
{
    Modem          modem {Serial1};
    adk::LoraModem lora  {Serial1, 7};

    adk::setup ();
    CHECK (lora.ok ());
    CHECK (Serial1.baud == 115200);
    CHECK (modem.commands.size () == 5);
    CHECK (modem.commands[0] == "AT");
    CHECK (modem.commands[1] == "AT+ADDRESS=7");
    CHECK (modem.commands[2] == "AT+NETWORKID=6");
    CHECK (modem.commands[3] == "AT+BAND=915000000");
    CHECK (modem.commands[4] == "AT+PARAMETER=10,7,1,7");
}

TEST (loraModemThatNeverAnswersIsNotOk)
{
    Modem          modem {Serial3, false};
    adk::LoraModem lora  {Serial3, 1, 3, 868000000};

    arduino::setClockStep (100);
    adk::setup ();
    CHECK (!lora.ok ());
    CHECK (modem.commands.size () > 1);
    CHECK (!lora.send (0, "Hello"));
}

TEST (loraModemSendsAndWaitsForOk)
{
    Modem          modem {Serial1};
    adk::LoraModem lora  {Serial1, 7};

    adk::setup ();
    Serial1.input.clear ();
    modem.commands.clear ();
    Serial1.onWrite = [&] (uint8_t byte)
    {
        modem.line += static_cast<char> (byte);
    };

    CHECK (lora.send (0, "Hello, you"));
    CHECK (modem.line == "AT+SEND=0,10,Hello, you\r\n");
    CHECK (lora.isSending ());
    CHECK (!lora.send (0, "Again"));

    adk::update (0);
    CHECK (lora.isSending ());
    Serial1.input = "+OK\r\n";
    adk::update (300);
    CHECK (!lora.isSending ());

    // A modem that never says +OK is given up on after 3 s.
    CHECK (lora.send (9, "Hi"));
    adk::update (400);
    adk::update (3399);
    CHECK (lora.isSending ());
    adk::update (3400);
    CHECK (!lora.isSending ());

    std::string tooLong (adk::LoraModem::MaxLength + 1, 'x');
    CHECK (!lora.send (0, tooLong.c_str ()));
}

TEST (loraModemHearsMessagesWithCommasInThem)
{
    Modem          modem {Serial1};
    adk::LoraModem lora  {Serial1, 7};

    adk::setup ();
    Serial1.input = "+RCV=50,10,Hi, there!,-99,-4\r\n+RCV=bad\r\n";
    adk::update (0);
    CHECK (lora.wasReceived ());
    CHECK (lora.sender () == 50);
    CHECK (std::string (lora.text ()) == "Hi, there!");
    CHECK (lora.signal () == -99);
    CHECK (lora.margin () == -4);

    adk::update (1);
    CHECK (!lora.wasReceived ());
    CHECK (std::string (lora.text ()) == "Hi, there!");

    Serial1.input = "+RCV=3,4,Hey,-40\r\n";
    adk::update (2);
    CHECK (!lora.wasReceived ());
}

TEST (loraLinkChoosesALawfulChannelAndPower)
{
    const adk::Pin Mode = 40;
    const adk::Pin Aux  = 41;
    Module         module {Serial3, Mode};
    adk::LoraLink  link   {Serial3, Mode, Aux};

    adk::setup ();
    CHECK (link.ok ());
    CHECK (Serial3.baud == 9600);
    CHECK (module.received == std::string ("\xC0\x00\x00\x1A\x18\x47", 6));

    for (bool inSettings : module.inSettings)
    {
        CHECK (inSettings);
    }

    CHECK (arduino::pin (Mode).mode == OUTPUT);
    CHECK (arduino::pin (Mode).output == LOW);
    CHECK (arduino::pin (Aux).mode == INPUT);
}

TEST (loraLinkAsksForItsSettingsWhenTheyAreNotRepeated)
{
    Module        module {Serial1, 42};
    adk::LoraLink link   {Serial1, 42, 43, 30};

    module.echo = false;
    Serial1.onWrite = [&] (uint8_t byte)
    {
        module.received += static_cast<char> (byte);

        if (module.received.size () == 9)
        {
            Serial1.input += std::string ("\xC0\x00\x00\x1A\x1E\x47", 6);
        }
    };

    arduino::setClockStep (100);
    adk::setup ();
    CHECK (link.ok ());
    CHECK (module.received.substr (6) == "\xC1\xC1\xC1");
}

TEST (loraLinkWithoutAModuleIsNotOk)
{
    adk::LoraLink link {Serial1, 42, 43};

    arduino::pin (43).input = LOW;
    arduino::setClockStep (100);
    adk::setup ();
    CHECK (!link.ok ());
    CHECK (!link.send ("Hello"));
}

TEST (loraLinkSendsAndReceivesLines)
{
    Module        module {Serial3, 40};
    adk::LoraLink link   {Serial3, 40, 41};

    adk::setup ();
    Serial3.text.clear ();
    Serial3.onWrite = nullptr;

    CHECK (link.send ("T 21.5 H 40"));
    CHECK (Serial3.text == "T 21.5 H 40\n");

    std::string tooLong (adk::LoraLink::MaxLength + 1, 'x');
    CHECK (!link.send (tooLong.c_str ()));

    Serial3.input = "Hello\nThere";
    adk::update (0);
    CHECK (link.wasReceived ());
    CHECK (std::string (link.text ()) == "Hello");
    adk::update (1);
    CHECK (!link.wasReceived ());
    Serial3.input = "\n";
    adk::update (2);
    CHECK (link.wasReceived ());
    CHECK (std::string (link.text ()) == "There");
}

TEST (meshNodeSendsAtMostEveryOneAndAHalfSeconds)
{
    adk::MeshNode mesh {Serial1};

    adk::setup ();
    CHECK (Serial1.baud == 38400);
    adk::update (1000);
    CHECK (mesh.canSend ());
    CHECK (mesh.send ("Lamp is on"));
    CHECK (Serial1.text == "Lamp is on");
    CHECK (!mesh.canSend ());
    CHECK (!mesh.send ("Too soon"));

    adk::update (2499);
    CHECK (!mesh.send ("Still too soon"));
    adk::update (2500);
    CHECK (mesh.send ("Now"));
    CHECK (Serial1.text == "Lamp is onNow");
}

TEST (meshNodeSplitsTheSenderFromTheMessage)
{
    adk::MeshNode mesh {Serial1};

    adk::setup ();
    Serial1.input = "\r\nPHNE: lamp on: please\r\n\r\nno name here\r\n";
    adk::update (0);
    CHECK (mesh.wasReceived ());
    CHECK (std::string (mesh.sender ()) == "PHNE");
    CHECK (std::string (mesh.text ()) == "lamp on: please");

    adk::update (1);
    CHECK (mesh.wasReceived ());
    CHECK (std::string (mesh.sender ()).empty ());
    CHECK (std::string (mesh.text ()) == "no name here");

    adk::update (2);
    CHECK (!mesh.wasReceived ());
    CHECK (std::string (mesh.text ()) == "no name here");
}
