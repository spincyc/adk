# The library

ADK is a small C++ library for the Arduino Mega 2560. A sketch declares one
object per part of its circuit, calls `adk::setup ()` once, and calls
`adk::update ()` at the top of every `loop ()`. Everything else is a
conversation with the parts.

```cpp
#include <Adk.h>

adk::Led     led     {26};
adk::Button  button  {22};
adk::Speaker speaker {10};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        led.toggle ();
        speaker.tone (adk::note::a4, 100);
    }
}
```

## The five calls

| Call | Where | What it does |
|---|---|---|
| `adk::setup ()` | once, in `setup ()` | Checks and prepares every part. |
| `adk::setup (Serial)` | instead of the above | The same, and prints what went wrong, if anything. |
| `adk::update ()` | top of `loop ()` | Lets every part do its work: debounce buttons, play notes, refresh displays, take readings. |
| `adk::wait (ms)` | anywhere in `loop ()` | Waits like `delay ()`, while every part keeps working. |
| `adk::stop ()` | anywhere | Puts every part in its safe state: lights off, sound and motors stopped. |

## Events

Some things happen at an instant: a button is pressed, a key typed, a
remote button received, a reading taken. Methods for those, such as
`button.wasPressed ()`, are true for exactly one `adk::update ()`: the one in
which it happened. Methods for how things are, such as
`button.isPressed ()`, are true for as long as it lasts.

## Faults

`adk::setup ()` checks every pin a part asks for. If a pin doesn't exist,
is used by two parts, or can't do what the part needs (only some pins can dim
an LED or drive a servo), setup stops before anything is switched on. Every
pin goes back to being a harmless input, and the Mega's own **L** LED blinks
the pin number: long flashes for tens, then short flashes for ones.

| You see | It means |
|---|---|
| 2 long, 6 short, pause, repeat | Pin 26 has a problem |
| 9 short, pause, repeat | Pin 9 has a problem |

For the full story, use `adk::setup (Serial)` and open the Serial Monitor at
9600 baud:

```text
adk: pin 9 needs a timer that is already in use
adk: a Speaker stops PWM on pins 9 and 10
```

## The parts

| Part | Class | For |
|---|---|---|
| LED | [`Led`](outputs.md#led) | on, off, toggle, blink |
| RGB LED | [`RgbLed`](outputs.md#rgbled) | any colour, and fades between them |
| Active buzzer | [`Buzzer`](outputs.md#buzzer) | on, off, beep |
| Passive buzzer | [`Speaker`](outputs.md#speaker) | notes and whole melodies |
| Relay | [`Relay`](outputs.md#relay) | switching a separate low-voltage circuit |
| Push button | [`Button`](inputs.md#button) | pressed, released, held |
| On/off sensor or switch | [`Switch`](inputs.md#switch) | PIR, tilt, reed, obstacle, flame, sound, touch |
| Potentiometer, light sensor | [`AnalogInput`](core.md#analoginput) | 0 to 1023, or scaled to any range |
| Thermistor | [`Thermistor`](sensors.md#thermistor) | temperature from a resistor that changes with heat |
| Keypad | [`Keypad`](inputs.md#keypad) | the key pressed, one at a time |
| Rotary encoder | [`RotaryEncoder`](inputs.md#rotaryencoder) | detents turned, either way |
| Joystick | [`Joystick`](inputs.md#joystick) | two axes, and directions for games |
| 74HC595 | [`ShiftRegister`](displays.md#shiftregister) | eight outputs from three pins |
| One digit | [`SevenSegment`](displays.md#sevensegment) | a digit or letter |
| Four digits | [`FourDigitDisplay`](displays.md#fourdigitdisplay) | numbers, words and times |
| LCD1602 | [`Lcd`](displays.md#lcd) | text, anything `Serial` can print |
| 8×8 matrix | [`LedMatrix`](displays.md#ledmatrix) | pixels, pictures and scrolling text |
| Servo | [`Servo`](motion.md#servo) | an angle, or a glide to one |
| Stepper | [`Stepper`](motion.md#stepper) | exact steps, at a set speed |
| DC motor | [`Motor`](motion.md#motor) | speed and direction through an L293D |
| Ultrasonic sensor | [`Ultrasonic`](sensors.md#ultrasonic) | distance in centimetres |
| DHT11 | [`Dht11`](sensors.md#dht11) | temperature and humidity |
| IR receiver | [`IrReceiver`](sensors.md#irreceiver) | the remote's button codes |
| 18B20 thermometer | [`Ds18b20`](sensors.md#ds18b20) | a precise temperature |
| RFID reader | [`Rfid`](sensors.md#rfid) | the card held to it |
| Real-time clock | [`Rtc`](sensors.md#rtc) | the date and time |
| Accelerometer | [`Mpu6050`](sensors.md#mpu6050) | tilt, acceleration and rotation |
| Plain pins | [`DigitalOutput`](core.md#digitaloutput), [`DigitalInput`](core.md#digitalinput), [`PwmOutput`](core.md#pwmoutput) | anything else |
| Helpers | [`Every`](core.md#every), [`Smoother`](core.md#smoother), [`Debouncer`](core.md#debouncer), [`Color`](outputs.md#color) | beats, smoothing and colours |

Each part's header in [`src/adk/`](https://github.com/spincyc/adk/tree/main/src/adk)
starts with how to wire it. [How ADK works](../ARCHITECTURE.md) explains the
design.
