# The Mega's 5V and GND feed the top rails. The relay sits above the board,
# its coil pins dropping into the rails and its signal from pin 11; the tap
# sensor lies below the Mega on three wires. The lamp is a circuit of its
# own: the 9 V battery's red lead to the relay's COM terminal, NO to a 1 kΩ
# resistor and a red LED in the top half, and back to the black lead.
bench = Bench ("A relay on pin 11 switching a 9 V battery, 1 kΩ resistor and LED, and a tap "
               "sensor on pin A12", columns=(1, 30))

bench.wire ("5V", "T+3")
bench.wire ("GND", "T-3")
bench.module ("sensor", name="tap", at=(1.4, 3.4), pins=["S", "+", "−"], label="tap sensor",
              facing="up")
bench.wire ("A12", "tap.S", color="yellow")
bench.wire ("5V", "tap.+")
bench.wire ("GND", "tap.−")
bench.module ("relay", at=(6.2, -1.55))
bench.wire ("11", "relay.S", color="green")
bench.wire ("T+10", "relay.+")
bench.wire ("T-11", "relay.−")
bench.module ("battery9v", name="battery", at=(8.3, -2.3))
bench.wire ("battery.+", "relay.COM")
bench.wire ("relay.NO", "j20", color="blue")
bench.resistor ("1 kΩ", "i20", "i24")
bench.led ("red", anode="h24", cathode="h25")
bench.wire ("battery.−", "j25")
bench.closeup (8, 28)
