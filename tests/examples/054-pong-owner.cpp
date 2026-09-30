#include "radio_fixture.h"

bool wentUp (const char* name, long& seen);
void catchBall ();
void moveBall ();
void sendOver ();
void showScore ();
void draw ();

#include "../../examples/lessons/054-radio-pong/Pong/Pong.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    expect (!firstServer && ball == Ball::There, "Board B begins without a ball");
    // Lose the initial zero-count packet entirely.
    arduino::drive (22, LOW);
    runFor (1);
    runFor (25);
    expect (ball == Ball::There, "Board B cannot serve alongside Board A");
    hear ("@ball=1:8018");
    expect (ball == Ball::Here && x == 5 && dx == -1, "incoming ball is mirrored");
    ball = Ball::There;
    // Lose crossing 2; crossing 3 repeats its column and drift.
    hear ("@ball=3:7691");
    expect (ball == Ball::Here && x == 4 && dx == 0
            && ballStep.period () == 240,
            "each crossing carries its own complete flight after loss");
    x = 7;
    y = 6;
    dx = 0;
    dy = 1;
    moveBall ();
    expect (ball == Ball::Serving && misses == 1, "only the player who misses serves");
    return result ();
}
