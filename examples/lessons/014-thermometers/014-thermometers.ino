// Lesson 14: Thermometers
// The DHT11, the thermistor and the 18B20 side by side on the LCD, updated
// every second.

#include <Adk.h>

adk::Lcd        lcd        {31, 32, 33, 34, 35, 36};
adk::Dht11      dht        {16};
adk::Thermistor thermistor {A2};
adk::Ds18b20    probe      {17};
adk::Every      refresh    {1000};

constexpr char degree = char (223);    // the LCD's own degree sign

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (refresh.ticked ())
    {
        showReadings ();
    }
}

// The spaces at the end of each row rub out what a longer number left.
void showReadings ()
{
    lcd.at (0, 0).print ("DHT11 ");
    showReading (dht.ok (), dht.temperature (), 0);
    adk::print (lcd, degree, "C  ");
    showReading (dht.ok (), dht.humidity (), 0);
    lcd.print ("%   ");

    lcd.at (0, 1).print ("NTC ");
    showReading (true, thermistor.celsius (), 1);
    lcd.print (" DS ");
    showReading (probe.ok (), probe.celsius (), 1);
    lcd.print ("   ");
}

// A reading with this many decimals, or two dashes while that thermometer
// has nothing good to say.
void showReading (bool ok, float value, int decimals)
{
    if (ok)
    {
        lcd.print (value, decimals);
    }
    else
    {
        lcd.print ("--");
    }
}
