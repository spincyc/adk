#include "radio_fixture.h"

bool wentUp (const char* name, long& seen);
void checkCard (uint32_t card);
void tell (const char* top, const char* bottom);
void showQuiet ();

#include "../../examples/lessons/051-doorbell-and-door/Inside/Inside.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@cards=0:0");
    hear ("@cards=1:305419896");
    expect (unlocked.isRunning (), "known card opens");
    runFor (1);
    runFor (5100);
    expect (!unlocked.isRunning (), "door closes after its timeout");

    // First unknown-card packet was lost; its repeat must not reuse Ada's UID.
    hear ("@cards=3:99");
    expect (!unlocked.isRunning (), "unknown repeated card does not reuse old UID");
    hear ("@cards=4:99");
    expect (!unlocked.isRunning (), "same unknown card stays unknown");
    hear ("@cards=0:0");
    expect (!unlocked.isRunning (), "sender reset is not a swipe");
    hear ("@cards=1:-1698898192");
    expect (unlocked.isRunning (), "signed 32-bit UID recovers Sam's card");
    return result ();
}
