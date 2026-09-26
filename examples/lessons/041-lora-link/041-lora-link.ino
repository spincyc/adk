// Lesson 41: LoRa Link
// Module A sends the DHT11's reading as a line of text every two seconds,
// or at once when the button is pressed; module B hears it, and the LCD
// shows it with how long ago it came.

#include <Adk.h>

adk::LoraLink  linkA   {Serial1, 40, 41};   // M0 and M1 on 40, AUX on 41
adk::LoraLink  linkB   {Serial3, 42, 43};   // M0 and M1 on 42, AUX on 43
adk::Dht11     dht     {16};
adk::Lcd       lcd     {31, 32, 33, 34, 35, 36};
adk::Button    button  {23};
adk::Every     refresh {250};
adk::Stopwatch sinceReport;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

    lcd.print (linkA.ok () ? "Module A ready" : "No reply from A");
    lcd.at (0, 1).print (linkB.ok () ? "Module B ready" : "No reply from B");
}

void loop ()
{
    adk::update ();

    if ((dht.measured () || button.wasPressed ()) && dht.ok ())
    {
        sendReport ();
    }

    if (linkB.wasReceived ())
    {
        showReport ();
    }

    if (refresh.ticked () && sinceReport.isRunning ())
    {
        adk::print (lcd.at (0, 1), sinceReport.elapsed () / 1000, " s ago   ");
    }
}

// The reading as text, such as "Temp 23C Hum 45%": a character for every
// digit, letter and space.
void sendReport ()
{
    adk::Text<32> report;
    adk::print (report, "Temp ", adk::fixed (dht.temperature (), 0),
                "C Hum ", adk::fixed (dht.humidity (), 0), '%');

    if (linkA.send (report.c_str ()))
    {
        adk::println (Serial, "A sends ", report.size (), " characters: ",
                      report.c_str ());
    }
}

// Exactly the characters A sent, on the top row.
void showReport ()
{
    sinceReport.restart ();
    adk::println (Serial, "B hears: ", linkB.text ());
    lcd.clear ();
    lcd.print (linkB.text ());
}
