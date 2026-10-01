#include "radio_fixture.h"

void takeKey (char key);

#include "../../examples/lessons/049-remote-keypad/Inside/Inside.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@1/0");
    hear ("@1/1 key=0:0");
    expect (typed.empty (), "a count of 0 is no key");
    hear ("@1/1 key=1:49");
    hear ("@1/1 key=2:50");
    hear ("@1/1 key=2:50");
    expect (typed.size () == 2 && typed[1] == '2', "the refresh is not another key");
    hear ("@1/1 key=3:50");
    expect (typed.size () == 3 && typed[2] == '2', "the same key again is a new key");

    // Board A restarts: its count starts again, and B hears its first key.
    hear ("@0/0 key=0:0");
    hear ("@1/1 key=1:42");
    expect (typed.empty (), "after A restarts, its first key arrives");

    // 9 then 4 typed too quickly: only the 4 arrives, and the code is wrong.
    hear ("@1/1 key=2:49 key=3:50 key=4:51");
    hear ("@1/1 key=6:52");
    hear ("@1/1 key=7:52");
    hear ("@1/1 key=8:35");
    expect (!unlocked && wrong == 1, "a lost key makes a wrong code, never a right one");

    hear ("@1/1 key=9:49");
    hear ("@1/1 key=10:50");
    hear ("@1/1 key=11:51");
    hear ("@1/1 key=12:52");
    hear ("@1/1 key=13:35");
    expect (unlocked && wrong == 0, "the right code opens");
    return result ();
}
