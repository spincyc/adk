#include "radio_fixture.h"

// The exercise replaces the sharing line in this actual lesson sketch.
#define loop heldButtonLoop
#include "../../examples/lessons/043-the-bridge/BoardA/BoardA.ino"
#undef loop

bool switchedOn = false;

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        switchedOn = !switchedOn;
    }

    bridge.share ("button", switchedOn);
    light.set (bridge.value ("button"));
}

void press ()
{
    arduino::drive (22, LOW);
    lesson_test::runFor (1);
    lesson_test::runFor (20);
    arduino::drive (22, HIGH);
    lesson_test::runFor (1);
    lesson_test::runFor (20);
}

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@1/0");
    hear ("@1/1 button=0");
    expect (!light.isOn (), "the initial zero leaves the receiver off");
    bridge.start ();    // send a baseline now, before the two presses
    runFor (120);
    Serial3.text.clear ();
    press ();
    press ();
    runFor (120);
    expect (!switchedOn && Serial3.text.find ("button=1") == std::string::npos,
            "two coalesced presses send the final off state");
    hear ("@1/1 button=0");
    expect (!light.isOn (), "losing the intermediate on packet still leaves the LED off");
    hear ("@1/1 button=1");
    expect (light.isOn (), "a single later press sets the LED on");
    runFor (5100);
    expect (!bridge.isConnected () && light.isOn (), "silence retains the last shared state");
    hear ("@1/1 button=0");
    expect (!light.isOn (), "reconnection applies the sender's current state");
    hear ("@1/1 button=1");
    hear ("@0/1 button=0");
    expect (!light.isOn (), "a restarted sender shares its initial off state");
    return result ();
}
