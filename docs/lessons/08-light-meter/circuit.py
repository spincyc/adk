# Laid out from the end of the breadboard nearest the Mega, in the order the
# signal travels: first the light sensor's divider in the bottom half, from
# the 5 V rail through the photoresistor to the point A1 reads, and on
# through 10 kΩ to the GND rail; then the five LEDs of the bar, pins 26 to 30
# in order, each from its pin into the LED, then through a resistor across
# the middle gap to the GND rail.
bench = Bench ("A photoresistor divider on A1, and five LEDs on pins 26 to 30")

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
