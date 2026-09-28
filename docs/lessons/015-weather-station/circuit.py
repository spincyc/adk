# Lesson 14's screen and DHT11 stay exactly where they were; the thermistor
# and the 18B20 go. Past the screen, at their homes beside it: the RGB LED
# on pins 5 to 7, the active buzzer on 12, and the alarm knob, its wiper
# reached by A0's wire round the bottom of the screen. The wires from the
# top header pass over the DHT11.
bench = Bench ("A weather station: the DHT11 on pin 16, the RGB LED on pins 5 to 7, the knob on A0 "
               "and the buzzer on pin 12, with the screen from Lesson 13", columns=(1, 62))

bench.screen (text=("23°C 45% Comfy", "Alarm at 30°C"))

bench.home_dht11 ()

bench.home_rgb_led (via=([(2.35, -1.55), (9.4, -1.55)],
                          [(2.25, -1.65), (9.7, -1.65)],
                          [(2.15, -1.75), (9.9, -1.75)]))

bench.home_buzzer ("active", via=[(1.60, -1.85), (10.4, -1.85)])

bench.home_knob ()

bench.closeup (21, 62)

# Readings to take with a multimeter: the alarm knob's wiper, and the red
# and green pins of the comfort light, which remember the mood.
bench.measure ("The alarm knob's wiper, on A0", red="A0", black="GND", expect="about 3.4 V",
               when="the screen says Alarm at 30°C")
bench.measure ("Pin 6, the light's green", red="6", black="GND", expect="about 5 V",
               when="Comfy, the light green")
bench.measure ("Pin 5, the light's red", red="5", black="GND", expect="about 5 V",
               when="Hot, even once it has cooled to 25 °C")
