# The tap sensor lies at its home below the left end of the Mega, where it
# stays in Lesson 36 beside the RFID reader, S on A12. The relay lies at its
# home above the board, S on pin 11. The lamp is a circuit of its own, laid
# out like any LED: NO into j13, the 1 kΩ resistor across the gap, the red
# LED in b13-b14, and the battery's black lead into a14. The battery stands
# beside the board, its terminals up: the black lead runs in just below row
# a, so the close-up shows where it goes, and the red lead rises to COM.
# Nothing in the lamp's circuit touches the rails.
bench = Bench ("A relay on pin 11 switching a 9 V battery, 1 kΩ resistor and LED, and a tap "
               "sensor on pin A12", columns=(1, 24))

bench.home_tap ()

bench.home_relay ()

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
