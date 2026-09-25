# The screen at its home, and the red LED on 3 at its home in column 38. The
# rotary encoder sits at its home above the Mega, its wires rising from 18,
# 19 and 22, its + from the inner 5V pin at the top of the long header and
# its GND from the GND beside pin 13; pin 3's wire goes over the encoder to
# the far end of the board.
bench = Bench ("An LCD on pins 31 to 36, a rotary encoder on 18 and 19 with its switch "
               "on 22, and a lamp on pin 3", columns=(1, 40))

bench.screen (text=(">Level   60%", " Mode    Steady"))

bench.module ("encoder", at=(3.0, -2.1))
bench.wire ("18", "encoder.CLK", via=[(3.55, 0.55), (3.2, 0.55)])
bench.wire ("19", "encoder.DT", via=[(3.65, 0.45), (3.3, 0.45)])
bench.wire ("22", "encoder.SW", via=[(4.3, 0.8), (4.3, 0.35), (3.4, 0.35)])
bench.wire ("5V.long", "encoder.+", via=[(4.25, 0.7), (4.25, 0.25), (3.5, 0.25)])
bench.wire ("GND.top", "encoder.GND", via=[(1.5, 0.15), (3.6, 0.15)])

bench.wire ("3", "j38", via=[(2.65, -2.5), (9.1, -2.5)])
bench.resistor ("220 Ω", "g38", "e38")
bench.led ("red", anode="b38", cathode="b39")
bench.wire ("a39", "B-39")
