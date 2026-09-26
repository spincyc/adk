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

constexpr adk::Note fanfare [] = {
    {adk::note::g4, 150}, {adk::note::c5, 150},
    {adk::note::e5, 150}, {adk::note::g5, 600},
};

enum class State { Idle, YourTurn };

State                     state = State::Idle;
adk::Vector<uint8_t, 100> sequence;     // Simon's keys, by number
uint8_t                   step = 0;     // how many you have repeated
int                       held = -1;    // the key you pressed, or -1
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

// Simon's turn: after a pause with the lights out, add one random key, then
// show the whole sequence.
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
        flash (keys[number]);
    }

    step  = 0;
    held  = -1;
    state = State::YourTurn;
}

void flash (Key& key)
{
    key.light.on ();
    speaker.tone (key.pitch);
    adk::wait (400);

    key.light.off ();
    speaker.stop ();
    adk::wait (150);
}

// Each light and tone follow its button, and letting go is your answer.
void yourTurn (int pressed)
{
    for (auto& key : keys)
    {
        key.light.set (key.button.isPressed ());
    }

    if (pressed >= 0)
    {
        speaker.tone (keys[pressed].pitch);
        held = pressed;
    }
    else if (held >= 0 && keys[held].button.wasReleased ())
    {
        speaker.stop ();
        check (held);
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

// A low buzz, a fanfare for a new best, and the lights blink again.
void gameOver (int score)
{
    speaker.tone (adk::note::c3, 1000);
    adk::wait (1500);

    if (score > best)
    {
        best = score;
        speaker.play (fanfare);
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
