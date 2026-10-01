// Lesson 06: Simon
// Repeat the growing sequence of lights and tones. How far can you go?

#include <Adk.h>

// Each key is a button, the light beside it and the tone they share.
struct Key
{
    adk::Button button;
    adk::Led    light;
    uint16_t    pitch;
};

adk::Array   keys    {Key {22, 26, adk::note::c4},     // red
                      Key {23, 27, adk::note::e4},     // yellow
                      Key {24, 28, adk::note::g4},     // green
                      Key {25, 29, adk::note::c5}};    // blue
adk::Speaker speaker {10};

enum class State { Idle, YourTurn };

State                     state = State::Idle;
adk::Vector<uint8_t, 100> sequence;     // Simon's keys, by number
uint8_t                   step = 0;     // how many you have repeated
int                       held = -1;    // the key you are holding, or -1
int                       best = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    randomSeed (analogRead (A7));
    waitForPlayer ();
}

void loop ()
{
    adk::update ();

    int pressed = pressedKey ();

    if (state == State::Idle && pressed >= 0)
    {
        sequence.clear ();
        nextRound ();
    }
    else if (state == State::YourTurn)
    {
        yourTurn (pressed);
    }
}

// All four lights blink until somebody presses a button.
void waitForPlayer ()
{
    for (auto& key : keys)
    {
        key.light.blink (1000);
    }

    state = State::Idle;
    Serial.println ("Press any button to play.");
}

// Simon's turn: a pause in the dark, one more random key, then the tune.
void nextRound ()
{
    for (auto& key : keys)
    {
        key.light.off ();
    }

    adk::wait (1000);
    sequence.push_back (random (4));

    for (auto number : sequence)
    {
        keys[number].light.on ();
        speaker.tone (keys[number].pitch, 400);
        adk::wait (400);
        keys[number].light.off ();
        adk::wait (150);
    }

    step  = 0;
    state = State::YourTurn;
}

// Hold a key to light it and hear it; letting go is your answer.
void yourTurn (int pressed)
{
    if (pressed >= 0 && held < 0)
    {
        held = pressed;
        keys[held].light.on ();
        speaker.tone (keys[held].pitch);
    }
    else if (held >= 0 && keys[held].button.wasReleased ())
    {
        keys[held].light.off ();
        speaker.stop ();
        check (held);
        held = -1;
    }
}

void check (int answer)
{
    if (answer != sequence[step])
    {
        gameOver (sequence.size () - 1);
    }
    else if (step < sequence.size () - 1)
    {
        step++;
    }
    else if (sequence.full ())
    {
        gameOver (sequence.size ());    // you have beaten Simon
    }
    else
    {
        nextRound ();
    }
}

// A low buzz, the score, and the lights blink again.
void gameOver (int score)
{
    speaker.tone (adk::note::c3, 1000);
    adk::wait (1500);

    if (score > best)
    {
        best = score;
    }

    adk::println (Serial, "You remembered ", score,
                  " steps. Best so far: ", best);
    waitForPlayer ();
}

// The number of the key pressed in this update, or -1 if none was.
int pressedKey ()
{
    for (int number = 0; number < 4; number++)
    {
        if (keys[number].button.wasPressed ())
        {
            return number;
        }
    }

    return -1;
}
