# The screen stays at the far end and button 23 at its home in columns
# 8 to 10. The 433 MHz receiver and transmitter stand in row j, columns
# 30 to 33 and 38 to 41, with no aerials fitted. The receiver on 43 runs
# from 5 V; the transmitter runs from 3.3 V and takes pin 46 through a
# 1 kΩ and 2 kΩ divider.
bench = Bench ("A 433 MHz receiver on pin 43 and transmitter on pin 46, a button on pin 23, and "
               "the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=("Message 3:", "Dinner's ready"))

bench.home_button ("23")

bench.home_rf_receiver ()

bench.home_rf_transmitter ()

bench.closeup (1, 63)

# Readings to take with a multimeter while nothing is being sent: the
# receiver's DATA flickering with noise, the transmitter's DAT resting at
# 0 V, and the transmitter's supply from the Mega's 3.3V pin.
bench.measure ("The receiver's DATA, hearing noise", red="43", black="GND", expect="about 2.5 V",
               when="nothing sending")
bench.measure ("The transmitter's DAT, resting", red="h39", black="GND", expect="0 V",
               when="nothing sending")
bench.measure ("The transmitter's supply", red="3.3V", black="GND", expect="about 3.3 V",
               when="any time")
