#include "radio_fixture.h"

void obey (uint8_t command, uint16_t from);
void showState ();

#include "../../examples/lessons/053-remote-lamp/Receiver/Receiver.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    obey (adk::remote::power, 1234);
    expect (!lampOn && presses == 1, "another remote's power code is forwarded");
    expect (button == adk::remote::power && address == 1234, "address is retained");
    runFor (1);
    runFor (120);
    expect (Serial3.text.find ("@0/0 lamp=0 press=1:315973") != std::string::npos,
            "count, address and command travel in one event record");
    obey (adk::remote::power, 0);
    expect (lampOn && presses == 1, "only kit power toggles the lamp");
    return result ();
}
