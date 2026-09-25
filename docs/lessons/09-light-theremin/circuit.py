# Lesson 8's light meter, kept exactly as it was: the five LEDs on pins 26
# to 30 at their homes in columns 6 to 30, and the light sensor's divider on
# A1 in column 40. Added at their homes: the passive buzzer on pin 10 across
# the middle gap in column 34, between the white LED and the divider, with
# its 220 Ω from a34 into the − rail; and the knob on A0 in e45 to e47, its
# left leg to the − rail and its right leg up to the top + rail.
bench = Bench ("Lesson 8's light meter, a passive buzzer on pin 10 and a knob on A0",
               columns=(1, 50))

# The five wires leave the double header as a ribbon and spread onto their
# lanes at once: 27 hops over 26 (the one crossing the header's paired rows
# make unavoidable) to reach j12 from above, while 28, 29 and 30 run below
# row j and rise into it. A1's wire runs below the board, clear of its edge.
lanes = {26: [(4.10, 1.00), (4.50, 1.00), (4.50, 1.05), (5.90, 1.05)],
         27: [(4.30, 1.05), (4.30, 0.90), (6.50, 0.90)],
         28: [(4.10, 1.10), (4.40, 1.10), (4.40, 1.15), (7.10, 1.15)],
         29: [(4.30, 1.15), (4.30, 1.25), (7.70, 1.25)],
         30: [(4.10, 1.30), (4.20, 1.30), (4.20, 1.35), (8.30, 1.35)]}
for pin, color, column in ((26, "red", 6), (27, "yellow", 12), (28, "green", 18),
                           (29, "blue", 24), (30, "white", 30)):
    bench.wire (str (pin), f"j{column}", via=lanes[pin])
    bench.resistor ("220 Ω", f"g{column}", f"e{column}")
    bench.led (color, anode=f"b{column}", cathode=f"b{column + 1}")
    bench.wire (f"a{column + 1}", f"B-{column + 1}")

bench.wire ("j40", "T+40")
bench.photoresistor ("f40", "e40")
bench.wire ("A1", "a40", via=[(2.30, 2.85), (9.25, 2.85)])
bench.resistor ("10 kΩ", "c40", "c43")
bench.wire ("a43", "B-43")

bench.wire ("10", "j34", via=[(1.90, 0.45), (8.65, 0.45)])
bench.buzzer ("f34", "e34", kind="passive")
bench.resistor ("220 Ω", "a34", "B-34")

bench.potentiometer ("e45", "e46", "e47")
bench.wire ("a45", "B-45")
bench.wire ("A0", "a46")
bench.wire ("d47", "T+49", via=[(10.05, 1.85), (10.25, 1.85), (10.25, 0.75)])
