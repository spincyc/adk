#include "radio_fixture.h"

void takeKey (char key);

#include "../../examples/lessons/049-remote-keypad/Inside/Inside.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@key=0:0");
    hear ("@key=1:49");
    // The first 2 (count 2) was lost. A repeat must still carry its own key.
    hear ("@key=3:50");
    expect (typed.empty (), "a sequence gap clears the partial entry");
    hear ("@key=4:50");
    expect (typed.size () == 1 && typed[0] == '2', "repeat uses its own payload");
    hear ("@key=0:0");
    expect (typed.empty (), "sender restart clears the partial entry");

    hear ("@key=1:49");
    hear ("@key=2:50");
    hear ("@key=3:51");
    // User typed 9 then 4 too quickly; the bridge retained only count 5.
    hear ("@key=5:52");
    hear ("@key=6:52");
    hear ("@key=7:35");
    expect (!unlocked, "123944# cannot open after the middle events coalesce");

    hear ("@key=0:0");
    hear ("@key=1:49");
    hear ("@key=2:50");
    hear ("@key=3:51");
    hear ("@key=4:52");
    hear ("@key=5:35");
    expect (unlocked, "a complete ordered correct entry still opens");
    return result ();
}
