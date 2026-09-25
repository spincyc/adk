// Lesson 03: Reaction Duel
// Wait for the light and beep, then press first. Too soon and you lose.

#include <Adk.h>

adk::Button redButton   {22};
adk::Button greenButton {23};
adk::Led    red         {26};
adk::Led    yellow      {27};
adk::Led    green       {28};
adk::Buzzer buzzer      {12};

enum State { Waiting, Ready, Go, Result };

State         state    = Waiting;
unsigned long readyAt  = 0;
unsigned long waitTime = 0;
unsigned long goAt     = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    randomSeed (analogRead (A7));

    yellow.blink (1000);
    Serial.println ("Reaction Duel! Press a button to start.");
}

void loop ()
{
    adk::update ();

    bool redPressed   = redButton.wasPressed ();
    bool greenPressed = greenButton.wasPressed ();

    if (state == Waiting || state == Result)
    {
        if (redPressed || greenPressed)
        {
            getReady ();
        }
    }
    else if (state == Ready)
    {
        if (redPressed)
        {
            falseStart ("Red", "Green", green);
        }
        else if (greenPressed)
        {
            falseStart ("Green", "Red", red);
        }
        else if (millis () - readyAt >= waitTime)
        {
            go ();
        }
    }
    else if (state == Go)
    {
        if (redPressed)
        {
            win ("Red", red);
        }
        else if (greenPressed)
        {
            win ("Green", green);
        }
    }
}

void getReady ()
{
    red.off ();
    yellow.off ();
    green.off ();

    readyAt  = millis ();
    waitTime = random (2000, 5000);
    state    = Ready;
    Serial.println ("Get ready...");
}

void go ()
{
    yellow.on ();
    buzzer.beep (200);

    goAt  = millis ();
    state = Go;
}

void win (const char* name, adk::Led& light)
{
    unsigned long reaction = millis () - goAt;

    Serial.print (name);
    Serial.print (" wins in ");
    Serial.print (reaction);
    Serial.println (" ms!");
    showWinner (light);
}

void falseStart (const char* cheat, const char* name, adk::Led& light)
{
    Serial.print (cheat);
    Serial.print (" pressed too soon, so ");
    Serial.print (name);
    Serial.println (" wins!");
    buzzer.beep (800);
    showWinner (light);
}

void showWinner (adk::Led& light)
{
    yellow.off ();
    light.blink (200);
    state = Result;

    // Let the loser's late press go by before a new round can start.
    adk::wait (1000);
    Serial.println ("Press a button to play again.");
}
