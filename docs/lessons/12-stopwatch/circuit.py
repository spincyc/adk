# Lesson 11's display left as it was, with the controls added beyond it:
# from the end of the breadboard nearest the Mega, the rails fed at that end;
# the 74HC595 with its supply and its wires from pins 37, 38 and 39; a 1 kΩ
# resistor from each output to its segment of the display, the bottom row
# of segments along the bottom half and the top row along the top half; the
# display's four digit pins wired straight to pins 40 to 43; then, past the
# display, the three buttons on pins 22, 23 and 24, each with its other side
# to GND, and last the active buzzer on pin 12 standing across the middle gap.
bench = Bench ("Lesson 11's display, three buttons on pins 22 to 24 and a buzzer on pin 12",
               columns=(1, 60))

bench.wire ("5V", "T+4")
bench.wire ("GND", "T-4")
bench.wire ("GND", "B-4")

bench.chip ("74HC595", first=6)
bench.wire ("j6", "T+6")
bench.wire ("37", "j8")
bench.wire ("j9", "T-9")
bench.wire ("39", "j10")
bench.wire ("38", "j11")
bench.wire ("j12", "T+12")
bench.wire ("a13", "B-13")

bench.four_digits (30, shows="12.34")

bench.resistor ("1 kΩ", "d9", "c30")
bench.resistor ("1 kΩ", "c8", "b31")
bench.resistor ("1 kΩ", "b12", "a32")
bench.resistor ("1 kΩ", "b7", "b33")
bench.resistor ("1 kΩ", "c11", "c34")
bench.resistor ("1 kΩ", "h7", "i31")
bench.wire ("a10", "g18")
bench.resistor ("1 kΩ", "h18", "i32")
bench.wire ("d6", "d5")
bench.wire ("e5", "g5")
bench.resistor ("1 kΩ", "h5", "j35")

bench.wire ("40", "j30")
bench.wire ("41", "j33")
bench.wire ("42", "j34")
bench.wire ("43", "a35")

for pin, column in ((22, 45), (23, 49), (24, 53)):
    bench.button (column)
    bench.wire (str (pin), f"j{column}")
    bench.wire (f"a{column + 2}", f"B-{column + 2}")

bench.wire ("12", "j59")
bench.buzzer ("f59", "e59")
bench.wire ("a59", "B-59")

# The close-up shows the stopwatch itself: the display, the buttons and the
# buzzer.
bench.closeup (22, 60)
