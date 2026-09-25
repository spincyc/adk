# The power module plugs into all four rails at the far end, both jumpers
# on 5 V; the Mega's GND joins them at B-3. The knob stands at its home in
# e45-e47, its wiper on A0 and its right leg fed from the top + rail. The
# servo lies below the board at its home, its plug under columns 52-54:
# power from the bottom rails, and its signal from pin 44.
bench = Bench ("A servo on pin 44 powered by the breadboard power module, and a knob on A0",
               columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")

bench.potentiometer ("e45", "e46", "e47")
bench.wire ("a45", "B-45")
bench.wire ("A0", "a46", via=[(2.2, 2.85), (9.9, 2.85)])
bench.wire ("d47", "T+49")

bench.module ("servo", "servo", at=(9.85, 3.45), facing="up")
bench.wire ("servo.+", "B+53")
bench.wire ("servo.−", "B-54")
bench.wire ("44", "servo.signal", via=[(4.55, 1.95), (4.55, 2.95), (10.5, 2.95)])

bench.closeup (38, 63)
