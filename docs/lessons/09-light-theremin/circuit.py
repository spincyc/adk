# Lesson 8's light meter, left as it was, with the knob and the buzzer added
# after it: from the end of the breadboard nearest the Mega, the light
# sensor's divider on A1, the five LEDs on pins 26 to 30, then the knob on A0
# between the GND and 5 V rails, and last the passive buzzer's loop: pin 10,
# the 220 Ω resistor, the buzzer standing across the middle gap, and down to
# the GND rail.
bench = Bench ("Lesson 8's light meter, a knob on A0 and a passive buzzer on pin 10",
               columns=(1, 40))

bench.wire ("5V", "B+3")
bench.wire ("GND", "B-3")
bench.wire ("B+4", "a4")
bench.photoresistor ("c4", "c6")
bench.wire ("A1", "a6")
bench.resistor ("10 kΩ", "b6", "b10")
bench.wire ("a10", "B-10")

for pin, color, column in ((26, "red", 13), (27, "yellow", 16), (28, "green", 19),
                           (29, "blue", 22), (30, "white", 25)):
    bench.wire (str (pin), f"j{column}")
    bench.led (color, anode=f"f{column}", cathode=f"f{column + 2}")
    bench.resistor ("220 Ω", f"g{column + 2}", f"e{column + 2}")
    bench.wire (f"a{column + 2}", f"B-{column + 2}")

bench.potentiometer ("e30", "e31", "e32")
bench.wire ("B-29", "a30")
bench.wire ("A0", "a31")
bench.wire ("a32", "B+33")

bench.wire ("10", "j34")
bench.resistor ("220 Ω", "h34", "h37")
bench.buzzer ("f37", "e37", kind="passive")
bench.wire ("a37", "B-37")
