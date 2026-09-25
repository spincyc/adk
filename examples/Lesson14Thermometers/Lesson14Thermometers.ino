// Lesson 14: Thermometers
// The DHT11, the thermistor and the 18B20 side by side on the LCD, updated every second.

#include <Adk.h>

adk::Lcd        lcd        {31, 32, 33, 34, 35, 36};
adk::Dht11      dht        {16};
adk::Thermistor thermistor {A2};
adk::Ds18b20    probe      {17};
adk::Every      refresh    {1000};

const uint8_t degree = 223;   // the degree sign in the LCD's own character set

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

void showReadings ()
{
    lcd.setCursor (0, 0);
    lcd.print ("DHT11 ");
    printReading (dht.ok (), dht.temperature (), 0);
    lcd.write (degree);
    lcd.print ("C  ");
    printReading (dht.ok (), dht.humidity (), 0);
    lcd.print ("%   ");

    lcd.setCursor (0, 1);
    lcd.print ("NTC ");
    printReading (true, thermistor.celsius (), 1);
    lcd.print (" DS ");
    printReading (probe.ok (), probe.celsius (), 1);
    lcd.print ("   ");
}

// A reading, or two dashes while that thermometer has nothing good to say.
void printReading (bool ok, float value, uint8_t decimals)
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
