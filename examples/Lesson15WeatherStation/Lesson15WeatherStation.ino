// Lesson 15: Weather Station
// Temperature and humidity on the LCD, a cold, comfy or hot light, and a heat alarm set by a knob.

#include <Adk.h>

adk::Lcd         lcd     {31, 32, 33, 34, 35, 36};
adk::Dht11       dht     {16};
adk::RgbLed      light   {5, 6, 7};
adk::AnalogInput knob    {A0};
adk::Buzzer      buzzer  {12};
adk::Every       refresh {250};
adk::Every       beat    {1000};

enum Comfort
{
    Cold,
    Comfy,
    Hot
};

// Comfy is from 18 to 25 degrees. The mood only changes once the temperature
// is a whole margin past a boundary, so a reading that wobbles on the edge
// can't make the light flicker.
const float comfyLow  = 18;
const float comfyHigh = 25;
const float margin    = 1;

const uint8_t degree = 223;

Comfort comfort = Comfy;
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
        checkAlarm ();
        showWeather ();
    }

    if (ringing && beat.ticked ())
    {
        buzzer.beep (150);
    }
}

void judgeComfort (float celsius)
{
    Comfort was = comfort;

    if (comfort == Comfy && celsius >= comfyHigh + margin)
    {
        comfort = Hot;
    }
    else if (comfort == Comfy && celsius <= comfyLow - margin)
    {
        comfort = Cold;
    }
    else if (comfort == Hot && celsius <= comfyHigh - margin)
    {
        comfort = Comfy;
    }
    else if (comfort == Cold && celsius >= comfyLow + margin)
    {
        comfort = Comfy;
    }

    if (comfort != was)
    {
        light.fadeTo (colorOf (comfort), 1000);
    }
}

// The alarm rings once it's as hot as the knob says, and stops once it's a
// margin cooler again, or the knob is turned up past the temperature. Until
// the first reading the temperature is 0, too cold to ring.
void checkAlarm ()
{
    float celsius = dht.temperature ();

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

void showWeather ()
{
    lcd.setCursor (0, 0);

    if (dht.ok ())
    {
        lcd.print (dht.temperature (), 0);
        lcd.write (degree);
        lcd.print ("C ");
        lcd.print (dht.humidity (), 0);
        lcd.print ("% ");
        lcd.print (nameOf (comfort));
    }
    else
    {
        lcd.print ("Measuring...    ");
    }

    lcd.setCursor (0, 1);
    lcd.print (ringing ? "TOO HOT! " : "Alarm at ");
    lcd.print (alarmAt);
    lcd.write (degree);
    lcd.print ("C   ");
}

adk::Color colorOf (Comfort mood)
{
    switch (mood)
    {
        case Cold: return adk::color::blue;
        case Hot:  return adk::color::red;
        default:   return adk::color::green;
    }
}

const char* nameOf (Comfort mood)
{
    switch (mood)
    {
        case Cold: return "Cold   ";
        case Hot:  return "Hot    ";
        default:   return "Comfy  ";
    }
}
