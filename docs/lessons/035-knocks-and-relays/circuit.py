# The tap sensor lies below the left end of the Mega, where it stays in
# Lesson 36 beside the RFID reader: + and − from the power header's 5V and
# GND, S from A12. The relay lies on its side above the board, its pins
# facing the Mega: S from pin 11 over the Mega, + and − from the long
# header's inner 5V and GND. The lamp is a circuit of its own, laid out like
# any LED: NO into j13, the 1 kΩ resistor across the gap, the red LED in
# b13-b14, and the battery's black lead into a14. The battery stands beside
# the board, its terminals up: the black lead runs in just below row a, so
# the close-up shows where it goes, and the red lead rises to COM. Nothing
# in the lamp's circuit touches the rails.
bench = Bench ("A relay on pin 11 switching a 9 V battery, 1 kΩ resistor and LED, and a tap "
               "sensor on pin A12", columns=(1, 24))

bench.module ("sensor", name="tap", at=(0.43, 3.62), pins=["S", "+", "−"], label="tap sensor",
              facing="up")
bench.wire ("A12", "tap.S", via=[(3.5, 2.95), (0.85, 2.95)])
bench.wire ("5V.power", "tap.+", via=[(1.7, 2.9), (0.75, 2.9)])
bench.wire ("GND.power", "tap.−", via=[(1.8, 2.85), (0.65, 2.85)])

bench.module ("relay", at=(5.05, -0.75), facing="left")
bench.wire ("11", "relay.S", via=[(1.8, -0.35)])
bench.wire ("5V.long", "relay.+", via=[(4.25, 0.7), (4.25, -0.25)])
bench.wire ("GND.long", "relay.−", via=[(4.35, 2.4), (4.35, -0.15)])

bench.wire ("relay.NO", "j13", via=[(6.6, -0.45)])
bench.resistor ("1 kΩ", "g13", "e13")
bench.led ("red", anode="b13", cathode="b14")
bench.module ("battery9v", name="battery", at=(8.2, 2.6), facing="up")
bench.wire ("battery.−", "a14", via=[(8.6, 2.28), (6.7, 2.28)])
bench.wire ("battery.+", "relay.COM")

bench.closeup (1, 24)

# Readings to take with a multimeter while the lamp is on, all in the
# lamp's own circuit.
bench.measure ("The battery, through the relay", red="h13", black="b14", expect="about 9 V",
               when="Lamp on")
bench.measure ("Across the resistor", red="g13", black="a13", expect="about 7 V", when="Lamp on")
bench.measure ("Across the LED", red="b13", black="b14", expect="about 2 V", when="Lamp on")
