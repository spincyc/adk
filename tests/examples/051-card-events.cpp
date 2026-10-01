#include "radio_fixture.h"

void checkCard (uint32_t card);
void tell (const char* top, const char* bottom);
void showQuiet ();

#include "../../examples/lessons/051-doorbell-and-door/Inside/Inside.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@2/0");
    hear ("@2/1 rings=0:0 knocks=0:0 cards=0:0");
    expect (!news.isRunning (), "counts of 0 are no news");
    hear ("@2/1 cards=1:305419896");
    expect (unlocked.isRunning (), "known card opens");
    runFor (1);
    runFor (5100);
    expect (!unlocked.isRunning (), "door closes after its timeout");

    // The first unknown-card message was lost; the next must not reuse
    // Ada's number.
    hear ("@2/1 cards=3:99");
    expect (!unlocked.isRunning (), "unknown card does not reuse the old number");
    hear ("@2/1 cards=3:99");
    expect (!unlocked.isRunning () && !bridge.changed ("cards"),
            "the refresh is not another swipe");

    // The door board restarts: nothing it counted before comes again, and
    // its first new swipe arrives.
    runFor (11000);
    hear ("@0/0 rings=0:0 knocks=0:0 cards=0:0");
    hear ("@1/1 rings=0:0 knocks=0:0 cards=0:0");
    expect (!unlocked.isRunning () && !news.isRunning (), "a restart is not a swipe");
    hear ("@1/1 cards=1:-1698898192");
    expect (unlocked.isRunning (), "signed 32-bit number recovers Sam's card");
    hear ("@1/1 rings=1:0");
    expect (news.isRunning (), "the bell rings");
    return result ();
}
