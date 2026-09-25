# Laid out from the end of the breadboard nearest the Mega: GND and 5 V onto the rails, the LCD in
# row a with its GND, 5 V and contrast wiring just to its left and the wires from 31 to 36
# dropping onto its pins, then the RGB LED, the buzzer across the middle gap and the IR receiver
# above the board; the PIR sensor sits below the Mega. A long wire joins the two − rails.
bench = Bench ("A room alarm: an LCD on 31 to 36, a PIR sensor on A12, an IR receiver on 2, an "
               "active buzzer on 12 and an RGB LED on 5, 6 and 7", columns=(1, 46))

bench.wire ("GND", "B-3")
bench.wire ("5V", "T+3")

bench.lcd (10, row="a", text=("ARMED", "Code:"))
bench.wire ("a6", "B-6")
bench.wire ("d6", "d10", color="black")
bench.resistor ("1 kΩ", "e6", "e12")
bench.wire ("c14", "c10", color="black")
bench.wire ("5V", "d11")
bench.wire ("31", "e13")
bench.wire ("32", "e15")
bench.wire ("33", "e20")
bench.wire ("34", "e21")
bench.wire ("35", "e22")
bench.wire ("36", "e23")
bench.resistor ("220 Ω", "d24", "g24")
bench.wire ("j24", "T+24")
bench.wire ("T-25", "e25")

bench.wire ("5", "j26")
bench.wire ("6", "j36")
bench.wire ("7", "j37")
bench.resistor ("220 Ω", "h26", "h30")
bench.resistor ("220 Ω", "g32", "g36")
bench.resistor ("220 Ω", "i33", "i37")
bench.rgb_led ("f30", "f31", "f32", "f33")
bench.wire ("j31", "T-31")

bench.wire ("12", "j40")
bench.buzzer ("f40", "e40")
bench.wire ("a40", "B-40")

bench.module ("ir_receiver", name="receiver", at=(9.29, -2.0))
bench.wire ("2", "receiver.S")
bench.wire ("receiver.+", "T+42")
bench.wire ("receiver.-", "T-43")
bench.wire ("T-45", "B-45")

bench.module ("pir", name="pir", at=(2.2, 3.5))
bench.wire ("A12", "pir.OUT", color="purple")
bench.wire ("pir.VCC", "5V")
bench.wire ("pir.GND", "GND")
