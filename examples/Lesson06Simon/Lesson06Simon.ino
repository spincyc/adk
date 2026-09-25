// Lesson 06: Simon
// Repeat the growing sequence of lights and tones. How far can you go?

#include <Adk.h>

// Each color by number: 0 is red, 1 yellow, 2 green and 3 blue.
adk::Button  buttons [] {{22}, {23}, {24}, {25}};
adk::Led     lights  [] {{26}, {27}, {28}, {29}};
adk::Speaker speaker {10};

const uint16_t tones [] = {adk::note::c4, adk::note::e4, adk::note::g4,
                           adk::note::c5};

const adk::Note fanfare [] = {
    {adk::note::g4, 150}, {adk::note::c5, 150},
    {adk::note::e5, 150}, {adk::note::g5, 600},
};

const int longest = 100;

enum State { Idle, Listening };

State   state = Idle;
uint8_t sequence [longest];
int     length = 0;         // steps in this round's sequence
int     step   = 0;         // how many the player has repeated so far
int     held   = -1;        // the button pressed on this turn, or -1
int     best   = 0;

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

    for (int color = 0; color < 4; color++)
    {
        if (state == Idle && buttons[color].wasPressed ())
        {
            newGame ();
        }
        else if (state == Listening)
        {
            followButton (color);
        }
    }
}

void waitForPlayer ()
{
    state = Idle;

    for (int color = 0; color < 4; color++)
    {
        lights[color].blink (1000);
    }

    Serial.println ("Press any button to play.");
}

void newGame ()
{
    setAll (false);
    state  = Listening;
    length = 0;
    adk::wait (1000);
    nextRound ();
}

// Add one random step, then show the whole sequence from the start.
void nextRound ()
{
    sequence[length] = random (4);
    length = length + 1;
    step   = 0;
    held   = -1;

    for (int index = 0; index < length; index++)
    {
        lights[sequence[index]].on ();
        speaker.tone (tones[sequence[index]]);
        adk::wait (400);

        lights[sequence[index]].off ();
        speaker.stop ();
        adk::wait (150);
    }
}

// The light and tone follow the button; letting go is the answer.
void followButton (int color)
{
    lights[color].set (buttons[color].isPressed ());

    if (buttons[color].wasPressed ())
    {
        speaker.tone (tones[color]);
        held = color;
    }
    else if (buttons[color].wasReleased () && color == held)
    {
        speaker.stop ();
        held = -1;
        check (color);
    }
}

void check (int color)
{
    if (color != sequence[step])
    {
        gameOver (length - 1);
    }
    else if (step + 1 < length)
    {
        step = step + 1;
    }
    else if (length < longest)
    {
        adk::wait (800);
        nextRound ();
    }
    else
    {
        gameOver (longest);
    }
}

void gameOver (int score)
{
    setAll (true);
    speaker.tone (adk::note::c3, 1000);
    adk::wait (1500);

    if (score > best)
    {
        best = score;
        speaker.play (fanfare);
    }

    Serial.print ("You remembered ");
    Serial.print (score);
    Serial.print (" steps. Best so far: ");
    Serial.println (best);
    waitForPlayer ();
}

void setAll (bool lit)
{
    for (int color = 0; color < 4; color++)
    {
        lights[color].set (lit);
    }
}
