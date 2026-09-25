# Lesson 13's screen stays in columns 1 to 21. After it, on the top half,
# come the RGB comfort light and the alarm buzzer, in the order their wires
# arrive from the top header. The alarm knob sits past the screen's
# overhang, fed from the bottom rails, which are joined to the top ones at
# the far end; the DHT11 module sits above that end.
bench = Bench ("A weather station: the DHT11 on pin 16, the RGB LED on pins 5 to 7, the knob on A0 "
               "and the buzzer on pin 12, with the LCD from Lesson 13", columns=(1, 55))

bench.potentiometer ("e1", "e2", "e3")
bench.lcd (5, row="a", text=("23°C 45% Comfy", "Alarm at 30°C"))

bench.wire ("5V", "T+3")
bench.wire ("GND", "a1")
bench.wire ("b1", "b5", color="black")
bench.wire ("e5", "T-5")
bench.wire ("e6", "T+6")
bench.wire ("d3", "d6", color="red")
bench.wire ("c2", "c7", color="brown")
bench.wire ("e9", "T-9")

bench.wire ("31", "e8")
bench.wire ("32", "e10")
bench.wire ("33", "e15")
bench.wire ("34", "e16")
bench.wire ("35", "e17")
bench.wire ("36", "e18")

bench.resistor ("220 Ω", "e19", "f19")
bench.wire ("j19", "T+19")
bench.wire ("e20", "T-21")

bench.rgb_led ("f25", "f26", "f27", "f28")
bench.wire ("5", "j22")
bench.resistor ("220 Ω", "h22", "h25")
bench.wire ("j26", "T-25")
bench.wire ("6", "j31")
bench.resistor ("220 Ω", "g27", "g31")
bench.wire ("7", "j35")
bench.resistor ("220 Ω", "h28", "h35")

bench.potentiometer ("e37", "e38", "e39")
bench.wire ("a37", "B+37")
bench.wire ("A0", "a38")
bench.wire ("a39", "B-39")

bench.buzzer ("f42", "f45")
bench.wire ("12", "j42")
bench.wire ("j45", "T-45")

bench.wire ("T+47", "B+47")
bench.wire ("T-48", "B-48")

bench.module ("dht11", "dht", at=(10.08, -1.25))
bench.wire ("16", "dht.S")
bench.wire ("dht.+", "T+51")
bench.wire ("dht.−", "T-52")

bench.closeup (18, 53)
