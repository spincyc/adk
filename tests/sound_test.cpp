#include "check.h"

#include <Arduino.h>

TEST (buzzerBeepsForItsDuration)
{
    adk::Buzzer buzzer {30};

    adk::setup ();
    buzzer.beep (200);
    CHECK (arduino::pin (30).output == HIGH);

    adk::update (1000);
    adk::update (1199);
    CHECK (buzzer.isOn ());

    adk::update (1200);
    CHECK (!buzzer.isOn ());
    CHECK (arduino::pin (30).output == LOW);
}

TEST (speakerSoundsAToneUntilStopped)
{
    adk::Speaker speaker {30};

    adk::setup ();
    speaker.tone (adk::note::a4);
    adk::update (100000);

    CHECK (arduino::pin (30).tone == 440);
    CHECK (speaker.isPlaying ());

    speaker.stop ();
    CHECK (arduino::pin (30).tone == 0);
    CHECK (!speaker.isPlaying ());
}

TEST (speakerToneEndsAfterItsDuration)
{
    adk::Speaker speaker {30};

    adk::setup ();
    speaker.tone (1000, 300);
    adk::update (0);
    adk::update (299);
    CHECK (arduino::pin (30).tone == 1000);

    adk::update (300);
    CHECK (arduino::pin (30).tone == 0);
    CHECK (!speaker.isPlaying ());
}

TEST (speakerPlaysAMelodyWithGapsBetweenNotes)
{
    const adk::Note tune [] = {{adk::note::c4, 400}, {adk::note::rest, 200}, {adk::note::g4, 400}};
    adk::Speaker    speaker {30};

    adk::setup ();
    speaker.play (tune);
    adk::update (0);
    CHECK (arduino::pin (30).tone == 262);

    adk::update (349);
    CHECK (arduino::pin (30).tone == 262);

    adk::update (350);
    CHECK (arduino::pin (30).tone == 0);

    adk::update (400);
    CHECK (arduino::pin (30).tone == 0);
    CHECK (speaker.isPlaying ());

    adk::update (600);
    CHECK (arduino::pin (30).tone == 392);

    adk::update (1000);
    CHECK (!speaker.isPlaying ());
}

TEST (speakerIsSilencedByStopAll)
{
    adk::Speaker speaker {30};

    adk::setup ();
    speaker.tone (440);
    adk::stop ();

    CHECK (arduino::pin (30).tone == 0);
}
