# Lesson 5's build stays as it is: the buttons on 22 to 25 and the passive
# buzzer on 10. The LEDs on 26 to 29 stand at their homes, each beside its
# button. Eight wires from the double header into interleaved columns can't
# all nest, so the buttons' wires fan out over the top of the board and drop
# straight into their columns through the gaps in the top rails, crossing
# the lanes of the ones going further; 26 comes in just above row j, and 27
# to 29 in nested lanes just below it.
bench = Bench ("Simon: buttons on pins 22 to 25, LEDs on 26 to 29, and a passive buzzer on 10",
               columns=(1, 40))

bench.home_button ("22", via=[(4.15, 0.75), (4.25, 0.75), (4.25, 0.15), (5.5, 0.15)])

bench.home_button ("23", via=[(4.35, 0.80), (4.35, 0.25), (6.1, 0.25)])

bench.home_button ("24", via=[(4.15, 0.85), (4.45, 0.85), (4.45, 0.35), (6.7, 0.35)])

bench.home_button ("25", via=[(4.55, 0.90), (4.55, 0.45), (7.3, 0.45)])

bench.home_led ("26", "red", via=[(4.15, 0.95), (4.65, 0.95), (4.65, 0.9), (5.9, 0.9)])

bench.home_led ("27", "yellow", via=[(4.45, 1.00), (4.45, 1.15), (6.4, 1.15)])

bench.home_led ("28", "green", via=[(4.15, 1.05), (4.35, 1.05), (4.35, 1.25), (7.0, 1.25)])

bench.home_led ("29", "blue", via=[(4.25, 1.10), (4.25, 1.35), (7.6, 1.35)])

bench.home_buzzer ("passive", via=[(1.80, 0.05), (8.65, 0.05)])

# Readings to take with a multimeter in your turn, while a button is held
# and its light stays on: each color keeps its own voltage.
bench.measure ("Across the red LED", red="b6", black="b7", expect="about 2 V",
               when="Red button held")
bench.measure ("Across the yellow LED", red="b12", black="b13", expect="about 2 V",
               when="Yellow button held")
bench.measure ("Across the green LED", red="b18", black="b19", expect="about 3.2 V",
               when="Green button held")
bench.measure ("Across the blue LED", red="b24", black="b25", expect="about 3.2 V",
               when="Blue button held")
