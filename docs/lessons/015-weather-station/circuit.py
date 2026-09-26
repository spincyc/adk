# Lesson 14's screen and DHT11 stay exactly where they were; the thermistor
# and the 18B20 go. Past the screen, at their homes beside it: the RGB LED
# in columns 41-46, its resistors across the gap and its common leg in the
# bottom − rail; the active buzzer across the gap in column 51; and the
# alarm knob in e57-e59, its wiper reached by A0's wire round the bottom
# of the screen. The wires from the top header pass over the DHT11.
bench = Bench ("A weather station: the DHT11 on pin 16, the RGB LED on pins 5 to 7, the knob on A0 "
               "and the buzzer on pin 12, with the screen from Lesson 13", columns=(1, 62))

bench.screen (text=("23°C 45% Comfy", "Alarm at 30°C"))

bench.module ("dht11", "dht", at=(8.58, -1.27))
bench.wire ("16", "dht.S")
bench.wire ("dht.+", "T+36")
bench.wire ("dht.−", "T-37")

bench.wire ("5", "j41", via=[(2.45, -1.55), (9.4, -1.55)])
bench.wire ("6", "j44", via=[(2.35, -1.65), (9.7, -1.65)])
bench.wire ("7", "j46", via=[(2.25, -1.75), (9.9, -1.75)])
bench.resistor ("220 Ω", "g41", "e41")
bench.resistor ("220 Ω", "g44", "e44")
bench.resistor ("220 Ω", "g46", "e46")
bench.rgb_led (red="a41", common="B-42", green="a44", blue="a46")

bench.wire ("12", "j51", via=[(1.7, -1.85), (10.4, -1.85)])
bench.buzzer ("f51", "e51", kind="active")
bench.wire ("a51", "B-51")

bench.potentiometer ("e57", "e58", "e59")
bench.wire ("a57", "B-57")
bench.wire ("A0", "a58")
bench.wire ("d59", "T+61")

bench.closeup (21, 62)

# Readings to take with a multimeter: the alarm knob's wiper, and the red
# and green pins of the comfort light, which remember the mood.
bench.measure ("The alarm knob's wiper, on A0", red="A0", black="GND", expect="about 3.4 V",
               when="the screen says Alarm at 30°C")
bench.measure ("Pin 6, the light's green", red="6", black="GND", expect="about 5 V",
               when="Comfy, the light green")
bench.measure ("Pin 5, the light's red", red="5", black="GND", expect="about 5 V",
               when="Hot, even once it has cooled to 25 °C")
