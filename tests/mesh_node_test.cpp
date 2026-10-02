#include "check.h"

#include <Arduino.h>
#include <string>

TEST (meshNodeKeepsTheWholePayloadApartFromItsSenderPrefix)
{
    adk::MeshNode mesh {Serial1};
    std::string payload (adk::MeshNode::MaxLength - 1, 'x');
    payload += '!';

    adk::setup ();

    for (const char* sender : {"PHNE", "abcdefghijklmnop", ""})
    {
        std::string prefix = sender;

        if (!prefix.empty ())
        {
            prefix += ": ";
        }

        Serial1.input = prefix + payload + "\r\n";
        adk::update (0);
        CHECK (mesh.wasReceived ());
        CHECK (std::string (mesh.sender ()) == sender);
        CHECK (std::string (mesh.text ()) == payload);
        CHECK (mesh.text ()[adk::MeshNode::MaxLength] == '\0');
    }
}

TEST (meshNodeTruncatesOnlyPayloadBeyondItsLimitAndReadsTheNextLine)
{
    adk::MeshNode mesh {Serial1};
    std::string payload (adk::MeshNode::MaxLength, 'x');

    adk::setup ();

    for (const char* prefix : {"PHNE: ", "", "abcdefghijklmnop: "})
    {
        Serial1.input = prefix + payload + "discard this suffix\nNEXT: intact\n";
        adk::update (0);
        CHECK (mesh.wasReceived ());
        CHECK (std::string (mesh.text ()) == payload);
        CHECK (mesh.text ()[adk::MeshNode::MaxLength] == '\0');
        adk::update (1);
        CHECK (mesh.wasReceived ());
        CHECK (std::string (mesh.sender ()) == "NEXT");
        CHECK (std::string (mesh.text ()) == "intact");
    }

    // A colon after a name too long to be a prefix stays in the payload.
    payload.replace (17, 2, ": ");
    Serial1.input = payload + "extra\n";
    adk::update (2);
    CHECK (mesh.wasReceived ());
    CHECK (std::string (mesh.sender ()).empty ());
    CHECK (std::string (mesh.text ()) == payload);
}
