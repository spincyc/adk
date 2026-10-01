#include "radio_fixture.h"

void catchBall ();
void moveBall ();
void showScore ();
void draw ();

#include "../../examples/lessons/054-radio-pong/Pong/Pong.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    expect (!firstServer && ball == Ball::There, "Board B begins without a ball");
    arduino::drive (22, LOW);
    runFor (1);
    runFor (25);
    expect (ball == Ball::There, "Board B cannot serve alongside Board A");

    // The boards meet, and the first message with A's ball in it is lost:
    // the ball still arrives.
    hear ("@1/0 misses=0");
    hear ("@1/1 ball=1:8018 misses=0");
    expect (ball == Ball::Here && x == 5 && dx == -1, "incoming ball is mirrored");
    ball = Ball::There;
    // Lose crossing 2; crossing 3 carries its own column and drift.
    hear ("@1/1 ball=3:7691 misses=0");
    expect (ball == Ball::Here && x == 4 && dx == 0
            && ballStep.period () == 240,
            "each crossing carries its own complete flight after loss");
    ball = Ball::There;
    hear ("@1/1 ball=3:7691 misses=0");
    expect (ball == Ball::There, "the refresh is not another crossing");

    // Board A restarts mid-rally: its old crossings never come back.
    hear ("@0/0 ball=0:0 misses=0");
    hear ("@1/1 ball=0:0 misses=0");
    expect (ball == Ball::There, "a restart does not replay an old crossing");

    x = 7;
    y = 6;
    dx = 0;
    dy = 1;
    ball = Ball::Here;
    moveBall ();
    expect (ball == Ball::Serving && misses == 1, "only the player who misses serves");
    return result ();
}
