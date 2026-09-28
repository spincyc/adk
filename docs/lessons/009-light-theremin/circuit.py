# Lesson 8's light meter, kept exactly as it was: the five LEDs on pins 26
# to 30 and the light sensor's divider on A1, at their homes. Added at their
# homes: the passive buzzer on pin 10, between the white LED and the
# divider, and the knob on A0.
bench = Bench ("Lesson 8's light meter, a passive buzzer on pin 10 and a knob on A0",
               columns=(1, 50))

# The five wires leave the double header as a ribbon and spread onto their
# lanes at once: 27 hops over 26 (the one crossing the header's paired rows
# make unavoidable) to reach j12 from above, while 28, 29 and 30 run below
# row j and rise into it. A1's wire runs below the board, clear of its edge.
lanes = {26: [(4.10, 0.95), (4.50, 0.95), (4.50, 1.05), (5.90, 1.05)],
         27: [(4.30, 1.00), (4.30, 0.90), (6.50, 0.90)],
         28: [(4.10, 1.05), (4.40, 1.05), (4.40, 1.15), (7.10, 1.15)],
         29: [(4.30, 1.10), (4.30, 1.25), (7.70, 1.25)],
         30: [(4.10, 1.25), (4.20, 1.25), (4.20, 1.35), (8.30, 1.35)]}
for pin, color in ((26, "red"), (27, "yellow"), (28, "green"), (29, "blue"), (30, "white")):
    bench.home_led (str (pin), color, via=lanes[pin])

bench.home_divider ("photoresistor")

bench.home_buzzer ("passive")

bench.home_knob ()

# Readings to take with a multimeter, with the sensor covered so that one
# note keeps sounding: pin 10's average, and the knob's wiper where the
# octave changes.
bench.measure ("Pin 10, playing a note", red="10", black="GND", expect="about 2.5 V",
               when="A note sounding")
bench.measure ("The knob's wiper, where the octave jumps", red="d40", black="GND",
               expect="about 1.7 V", when="The note just jumping up an octave")
