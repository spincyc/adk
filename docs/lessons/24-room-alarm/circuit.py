# The course's screen at its home, from column 5, as in Lesson 13. Past it,
# the parts that share a lesson with the screen stand at their second
# homes: the RGB LED in columns 41-46 and the active buzzer on 12 in column
# 51. The IR receiver sits above the board between the screen's wires and
# the LED's, taking its power from the top rails below it; the PIR sits
# below the Mega, as in Lesson 23, powered from the Mega's own 5V and GND.
bench = Bench ("A room alarm: an LCD on 31 to 36, a PIR sensor on A12, an IR receiver on 2, an "
               "active buzzer on 12 and an RGB LED on 5, 6 and 7", columns=(1, 55))

bench.screen (text=("ARMED", "Code:"))

bench.module ("ir_receiver", name="receiver", at=(7.89, -1.7))
bench.wire ("2", "receiver.S", via=[(2.75, -0.2), (8.1, -0.2)])
bench.wire ("receiver.+", "T+29")
bench.wire ("receiver.−", "T-30")

bench.wire ("5", "j41", via=[(2.45, -2.0), (9.4, -2.0)])
bench.wire ("6", "j44", via=[(2.35, -2.1), (9.7, -2.1)])
bench.wire ("7", "j46", via=[(2.25, -2.2), (9.9, -2.2)])
bench.resistor ("220 Ω", "g41", "e41")
bench.resistor ("220 Ω", "g44", "e44")
bench.resistor ("220 Ω", "g46", "e46")
bench.rgb_led (red="a41", common="B-42", green="a44", blue="a46")

bench.wire ("12", "j51", via=[(1.69, -2.3), (10.4, -2.3)])
bench.buzzer ("f51", "e51", kind="active")
bench.wire ("a51", "B-51")

bench.module ("pir", name="pir", at=(1.26, 3.8), facing="up")
bench.wire ("A12", "pir.OUT", via=[(3.49, 3.05), (1.89, 3.05)])
bench.wire ("5V.power", "pir.VCC", via=[(1.69, 2.9), (1.99, 2.9)])
bench.wire ("GND.power", "pir.GND")
