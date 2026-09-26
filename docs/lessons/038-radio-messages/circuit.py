# The screen at its home, and the push button on 23 at its home beside it,
# in columns 38 to 40. The 433 MHz receiver and transmitter stand in row j
# past it, their boards over the top rails and their springs pointing up,
# with every wire coming up from below, round the bottom of the screen.
# The receiver, in columns 48 to 51, takes 5 V from the Mega's power header
# into VCC's column, pin 43 into DATA's, and a black jumper takes GND down
# to the bottom − rail. The transmitter, in columns 56 to 59, takes the
# Mega's 3.3V into its + column and GND the same way; its DAT is the middle
# of a divider: pin 46 into f54, 1 kΩ along row h to DAT's column, and
# 2 kΩ across the gap and down to the − rail.
bench = Bench ("A 433 MHz receiver on pin 43 and transmitter on pin 46, a button on pin 23, and "
               "the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=("Message 3:", "Dinner's ready"))

bench.wire ("23", "j38", via=[(4.4, 0.85), (4.4, -1.7), (9.1, -1.7)])
bench.button (38)
bench.wire ("a40", "B-40")

bench.header_module ("rf_receiver", first=48, row="j")
bench.wire ("5V.power", "f48", via=[(1.69, 3.65), (10.1, 3.65)])
bench.wire ("43", "f49", via=[(4.45, 1.85), (4.45, 3.75), (10.2, 3.75)])
bench.wire ("f51", "B-51")

bench.header_module ("rf_transmitter", first=56, row="j")
bench.wire ("46", "f54", via=[(4.25, 2.0), (4.55, 2.0), (4.55, 3.85), (10.7, 3.85)])
bench.resistor ("1 kΩ", "h54", "h57")
bench.resistor ("2 kΩ", "g57", "e57")
bench.wire ("a57", "B-57")
bench.wire ("3.3V", "f58", via=[(1.59, 3.95), (11.1, 3.95)])
bench.wire ("f59", "B-59")

bench.closeup (23, 62)

# Readings to take with a multimeter while nothing is being sent: the
# receiver's DATA flickering with noise, the transmitter's DAT resting at
# 0 V, and the transmitter's supply from the Mega's 3.3V pin.
bench.measure ("The receiver's DATA, hearing noise", red="43", black="GND", expect="about 2.5 V",
               when="nothing sending")
bench.measure ("The transmitter's DAT, resting", red="h57", black="GND", expect="0 V",
               when="nothing sending")
bench.measure ("The transmitter's supply", red="3.3V", black="GND", expect="about 3.3 V",
               when="any time")
