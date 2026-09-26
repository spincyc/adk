// Lesson 15: Weather Station
// Temperature and humidity on the LCD, a cold, comfy or hot light, and a
// heat alarm set with a knob.

#include <Adk.h>

adk::Lcd         lcd     {31, 32, 33, 34, 35, 36};
adk::Dht11       dht     {16};
adk::RgbLed      light   {5, 6, 7};
adk::AnalogInput knob    {A0};
adk::Buzzer      buzzer  {12};
adk::Every       refresh {250};     // the knob, the alarm and the screen
adk::Every       beat    {1000};    // the alarm's beeps

enum class Comfort { Cold, Comfy, Hot };

// Comfy is from 18 to 25 degrees. The mood only changes once the
// temperature is a whole margin past a boundary, so a reading that wobbles
// on the edge can't make the light flicker.
constexpr float comfyLow  = 18;
constexpr float comfyHigh = 25;
constexpr float margin    = 1;

constexpr char degree = char (223);    // the LCD's own degree sign

Comfort comfort = Comfort::Comfy;
long    alarmAt = 30;
bool    ringing = false;

void setup ()
{
    adk::setup ();

    light.show (colorOf (comfort));
}

void loop ()
{
    adk::update ();

    if (dht.measured () && dht.ok ())
    {
        judgeComfort (dht.temperature ());
    }

    if (refresh.ticked ())
    {
        alarmAt = knob.read (10, 40);
        checkAlarm (dht.temperature ());
        showWeather ();
    }

    if (ringing && beat.ticked ())
    {
        buzzer.beep (150);
    }
}

// The rules in the lesson's table: the mood changes only from where it is.
// Asking the light for the color it already shows, or is already fading
// to, changes nothing, so it only fades when the mood has changed.
void judgeComfort (float celsius)
{
    if (comfort == Comfort::Comfy && celsius >= comfyHigh + margin)
    {
        comfort = Comfort::Hot;
    }
    else if (comfort == Comfort::Comfy && celsius <= comfyLow - margin)
    {
        comfort = Comfort::Cold;
    }
    else if (comfort == Comfort::Hot && celsius <= comfyHigh - margin)
    {
        comfort = Comfort::Comfy;
    }
    else if (comfort == Comfort::Cold && celsius >= comfyLow + margin)
    {
        comfort = Comfort::Comfy;
    }

    light.fadeTo (colorOf (comfort), 1000);
}

// The alarm rings once it's as hot as the knob says, and stops once it's a
// margin cooler again, or the knob is turned up past the temperature.
// Until the first reading the temperature is 0, too cold to ring.
void checkAlarm (float celsius)
{
    if (!ringing && celsius >= alarmAt)
    {
        ringing = true;
        beat.restart ();
        buzzer.beep (150);
    }
    else if (ringing && celsius <= alarmAt - margin)
    {
        ringing = false;
    }
}

// The names are padded with spaces, so a short one rubs out a longer one.
void showWeather ()
{
    lcd.at (0, 0);

    if (dht.ok ())
    {
        adk::print (lcd, adk::fixed (dht.temperature (), 0), degree, "C ",
                    adk::fixed (dht.humidity (), 0), "% ", nameOf (comfort));
    }
    else
    {
        lcd.print ("Measuring...    ");
    }

    adk::print (lcd.at (0, 1), ringing ? "TOO HOT! " : "Alarm at ",
                alarmAt, degree, "C   ");
}

adk::Color colorOf (Comfort mood)
{
    switch (mood)
    {
        case Comfort::Cold: return adk::color::blue;
        case Comfort::Hot:  return adk::color::red;
        default:            return adk::color::green;
    }
}

const char* nameOf (Comfort mood)
{
    switch (mood)
    {
        case Comfort::Cold: return "Cold   ";
        case Comfort::Hot:  return "Hot    ";
        default:            return "Comfy  ";
    }
}
