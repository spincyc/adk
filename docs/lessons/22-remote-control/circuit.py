# Laid out from the end of the breadboard nearest the Mega: 5 V and GND onto the rails, then the
# RGB LED low on the board, its three resistors fanning up to where the wires from 5, 6 and 7
# drop in, and its longest leg in the − rail. The IR receiver sits on jumpers above the board,
# powered from the top rails.
bench = Bench ("An IR receiver on pin 2, and an RGB LED on pins 5, 6 and 7, each color through "
               "a 220 Ω resistor", columns=(1, 30))

bench.wire ("5V", "T+3")
bench.wire ("GND", "T-4")
bench.wire ("GND", "B-3")

bench.wire ("5", "j6")
bench.wire ("6", "j10")
bench.wire ("7", "j13")
bench.resistor ("220 Ω", "g6", "e8")
bench.resistor ("220 Ω", "g10", "e10")
bench.resistor ("220 Ω", "g13", "e11")
bench.rgb_led ("a8", "B-9", "a10", "a11")

bench.module ("ir_receiver", name="receiver", at=(6.7, -2.0))
bench.wire ("2", "receiver.S", color="white")
bench.wire ("receiver.+", "T+18")
bench.wire ("receiver.-", "T-19")
bench.closeup (1, 30)
