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

TEST (speakerPlaysARecordedVector)
{
    adk::Vector<adk::Note, 8> recorded;
    adk::Speaker              speaker {30};

    recorded.push_back ({adk::note::e4, 200});
    recorded.push_back ({adk::note::g4, 200});

    adk::setup ();
    speaker.play (recorded);
    adk::update (0);
    CHECK (arduino::pin (30).tone == 330);

    adk::update (200);
    CHECK (arduino::pin (30).tone == 392);

    adk::update (400);
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

TEST (buzzerClaimsItsPinAndStartsOff)
{
    adk::Buzzer buzzer {30};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (30).mode == OUTPUT);
    CHECK (arduino::pin (30).output == LOW);
    CHECK (!buzzer.isOn ());
}

TEST (buzzerPinCannotBeUsedTwice)
{
    adk::Buzzer buzzer {30};
    adk::Led    led    {30};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 30);
}

TEST (activeLowBuzzerSoundsWhenItsPinIsLow)
{
    adk::Buzzer buzzer {30, adk::ActiveLow};

    adk::setup ();
    CHECK (arduino::pin (30).output == HIGH);

    buzzer.beep (100);
    CHECK (arduino::pin (30).output == LOW);

    adk::update (0);
    adk::update (100);
    CHECK (arduino::pin (30).output == HIGH);
    CHECK (!buzzer.isOn ());
}

TEST (beepingAgainWhileABeepSoundsKeepsItsLength)
{
    adk::Buzzer buzzer {30};

    adk::setup ();
    buzzer.beep (200);
    adk::update (0);

    adk::update (150);
    buzzer.beep (200);
    adk::update (199);
    buzzer.beep (200);
    CHECK (buzzer.isOn ());

    adk::update (200);
    CHECK (!buzzer.isOn ());

    // Asked again once it has ended, it beeps again.
    buzzer.beep (200);
    CHECK (buzzer.isOn ());
}

TEST (aBeepOfAnotherLengthStartsAfresh)
{
    adk::Buzzer buzzer {30};

    adk::setup ();
    buzzer.beep (200);
    adk::update (0);

    adk::update (150);
    buzzer.beep (100);
    adk::update (160);
    adk::update (259);
    CHECK (buzzer.isOn ());

    adk::update (260);
    CHECK (!buzzer.isOn ());

    buzzer.on ();
    buzzer.beep (0);
    CHECK (!buzzer.isOn ());
}

TEST (stoppedBuzzerIsSilentAndForgetsItsBeep)
{
    adk::Buzzer buzzer {30};

    adk::setup ();
    buzzer.beep (500);
    adk::update (0);

    adk::stop ();
    CHECK (!buzzer.isOn ());
    CHECK (arduino::pin (30).output == LOW);

    buzzer.on ();
    adk::update (600);
    CHECK (buzzer.isOn ());
}

TEST (speakerPlaysTonesLongerThanSixtyFiveSeconds)
{
    adk::Speaker speaker {30};

    adk::setup ();
    speaker.tone (440, 70000);
    adk::update (0);
    adk::update (69999);
    CHECK (speaker.isPlaying ());

    adk::update (70000);
    CHECK (!speaker.isPlaying ());

    speaker.tone (440, 65536);
    adk::update (100000);
    adk::update (165535);
    CHECK (speaker.isPlaying ());

    adk::update (165536);
    CHECK (!speaker.isPlaying ());
    CHECK (arduino::pin (30).tone == 0);
}

TEST (speakerRestIsSilenceForItsDurationOrAStop)
{
    adk::Speaker speaker {30};

    adk::setup ();
    speaker.tone (440);
    speaker.tone (adk::note::rest, 300);
    CHECK (arduino::pin (30).tone == 0);
    CHECK (speaker.isPlaying ());

    adk::update (0);
    adk::update (300);
    CHECK (!speaker.isPlaying ());

    speaker.tone (440);
    speaker.tone (adk::note::rest);
    CHECK (arduino::pin (30).tone == 0);
    CHECK (!speaker.isPlaying ());
}

TEST (speakerToneAskedForAgainKeepsItsLength)
{
    adk::Speaker speaker {30};

    adk::setup ();
    speaker.tone (1000, 300);
    adk::update (0);

    speaker.tone (1000, 300);
    adk::update (200);
    speaker.tone (1000, 300);
    adk::update (299);
    CHECK (arduino::pin (30).tone == 1000);

    adk::update (300);
    CHECK (!speaker.isPlaying ());

    // A new pitch starts at once.
    speaker.tone (440);
    speaker.tone (880);
    CHECK (arduino::pin (30).tone == 880);
}

TEST (speakerMelodyAskedForOnEveryPassPlaysThrough)
{
    const adk::Note tune [] = {{adk::note::c4, 100}, {adk::note::e4, 100}};
    adk::Speaker    speaker {30};

    adk::setup ();

    for (adk::Millis now = 0; now <= 150; now += 10)
    {
        speaker.play (tune);
        adk::update (now);
    }

    CHECK (arduino::pin (30).tone == 330);
}

TEST (speakerSkipsNotesOfNoLength)
{
    const adk::Note tune [] = {{adk::note::c4, 0},
                               {adk::note::d4, 100},
                               {adk::note::e4, 0},
                               {adk::note::f4, 100},
                               {adk::note::g4, 0}};
    adk::Speaker    speaker {30};

    adk::setup ();
    speaker.play (tune);
    adk::update (0);
    CHECK (arduino::pin (30).tone == 294);

    adk::update (100);
    CHECK (arduino::pin (30).tone == 349);

    adk::update (200);
    CHECK (!speaker.isPlaying ());
}

TEST (speakerPlaysNothingForAnEmptyMelody)
{
    const adk::Note           silence [] = {{adk::note::c4, 0}};
    adk::Vector<adk::Note, 4> empty;
    adk::Speaker              speaker {30};

    adk::setup ();
    speaker.tone (440);
    speaker.play (empty);
    CHECK (!speaker.isPlaying ());
    CHECK (arduino::pin (30).tone == 0);

    speaker.tone (440);
    speaker.play (silence);
    CHECK (!speaker.isPlaying ());
    CHECK (arduino::pin (30).tone == 0);

    speaker.tone (440);
    speaker.play ({nullptr, 0});
    CHECK (!speaker.isPlaying ());
}

namespace {

    // Whether speaker.play () accepts a melody of this kind.
    template <typename Melody>
    concept Playable = requires (adk::Speaker& speaker, Melody&& melody)
    {
        speaker.play (static_cast<Melody&&> (melody));
    };
}

// A melody made on the spot would be gone before it had played, so it does
// not compile; one that lasts does.
static_assert (Playable<const adk::Note (&) [2]>);
static_assert (Playable<adk::Array<adk::Note, 2>&>);
static_assert (Playable<adk::Vector<adk::Note, 2>&>);
static_assert (!Playable<adk::Note (&&) [2]>);
static_assert (!Playable<adk::Array<adk::Note, 2>>);
static_assert (!Playable<adk::Vector<adk::Note, 2>>);
