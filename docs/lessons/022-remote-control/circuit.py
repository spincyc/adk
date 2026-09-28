# The RGB LED stands at its home, as in Lesson 4, on pins 5, 6 and 7. The
# IR receiver sits above the gap between the Mega and the breadboard, under
# the LED's wires: its signal to pin 2, its power from the Mega's 5V and GND
# at the ends of the long header.
bench = Bench ("An IR receiver on pin 2, and an RGB LED on pins 5, 6 and 7, each color through "
               "a 220 Ω resistor", columns=(1, 30))

bench.home_rgb_led (via=([(2.35, -1.15), (5.9, -1.15)],
                          [(2.25, -1.25), (6.2, -1.25)],
                          [(2.15, -1.35), (6.4, -1.35)]))

bench.module ("ir_receiver", name="receiver", at=(4.3, -0.9))
bench.wire ("2", "receiver.S")
bench.wire ("receiver.+", "5V.long")
bench.wire ("receiver.−", "GND.long")

bench.closeup (1, 30)

# Readings to take with a multimeter while the lamp holds a color.
bench.measure ("The red pin, full brightness", red="5", black="GND", expect="about 5 V",
               when="after pressing 1")
bench.measure ("The red pin, four presses dimmer", red="5", black="GND", expect="about 2.5 V",
               when="after 1, then VOL− four times")
bench.measure ("Across the blue LED", red="b11", black="GND", expect="about 3.2 V",
               when="after pressing 3, full brightness")
