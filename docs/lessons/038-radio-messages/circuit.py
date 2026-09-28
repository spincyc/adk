# The screen at its home, and the push button on 23 at its home beside it.
# The 433 MHz receiver and transmitter stand at their homes past it, with
# no aerials fitted. The receiver, on pin 43, runs from 5 V; the
# transmitter runs from the Mega's 3.3V, and its DAT is the middle of a
# divider from pin 46.
bench = Bench ("A 433 MHz receiver on pin 43 and transmitter on pin 46, a button on pin 23, and "
               "the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=("Message 3:", "Dinner's ready"))

bench.home_button ("23", via=[(4.4, 0.80), (4.4, -1.7), (9.1, -1.7)])

bench.home_rf_receiver ()

bench.home_rf_transmitter ()

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
