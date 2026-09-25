# Laid out from the end of the breadboard nearest the Mega: 5 V onto the top + rail and GND
# onto the bottom − rail, the sensor above the Mega with Trig and Echo straight over pins 14 and
# 15, then the gauge in the order its wires leave the header (green on 28 first, so no wire
# crosses another), the buzzer on 12 at the far end, and a long wire joining the two − rails.
bench = Bench ("An ultrasonic sensor on pins 14 and 15, green, yellow and red LEDs on 28, 27 "
               "and 26, and an active buzzer on 12", columns=(1, 34))

bench.module ("ultrasonic", name="sensor", at=(2.315, -1.26))
bench.wire ("14", "sensor.Trig")
bench.wire ("15", "sensor.Echo")
bench.wire ("5V", "T+3")
bench.wire ("sensor.VCC", "T+4")
bench.wire ("sensor.GND", "T-5")
bench.wire ("GND", "B-3")

for pin, color, column in (("28", "green", 6), ("27", "yellow", 13), ("26", "red", 20)):
    bench.wire (pin, f"e{column}")
    bench.led (color, anode=f"a{column}", cathode=f"a{column + 1}")
    bench.resistor ("220 Ω", f"e{column + 1}", f"e{column + 5}")
    bench.wire (f"a{column + 5}", f"B-{column + 5}")

bench.wire ("12", "j27")
bench.buzzer ("h27", "h30")
bench.wire ("j30", "T-31")
bench.wire ("T-33", "B-33")
