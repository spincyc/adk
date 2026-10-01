# Lesson 41's screen, button, DHT11 and Serial1 divider stay. Both LoRa
# modules, B's divider and the power module go. The Meshtastic board lies
# below the former module A, far enough down that its plugs clear the
# LCD's edge. GPIO48 reaches f35, GPIO47 reaches the divider at c37, and
# GND reaches the bottom − rail by column 41. Its own USB cable powers it.
# The RGB LED keeps its usual legs in a6, a9 and a11; button 23 covers
# the green resistor's usual hole e9, so that resistor stands in column 13
# and a short jumper joins b13 to b9.
bench = Bench ("A Meshtastic board on pins 18 and 19, its GPIO47 through a 1 kΩ and 2 kΩ divider "
               "and powered from its own USB cable; the RGB LED lamp on pins 5 to 7, the DHT11 on "
               "pin 16, a button on 23 and the LCD on pins 31 to 36", columns=(1, 63))

bench.screen (text=("4f2a says:", "Hi Mega!"))

bench.home_dht11 ()

bench.home_rgb_led (green_resistor=13)

bench.module ("mesh_board", "node", at=(7.4, 4.35), facing="up")
bench.wire ("18", "j37")
bench.resistor ("1 kΩ", "g37", "e37")
bench.resistor ("2 kΩ", "a37", "B-37")
bench.wire ("node.47", "c37", color="white")
bench.wire ("node.48", "f35", color="grey")
bench.wire ("19", "j35")
bench.wire ("node.GND", "B-41")

bench.home_button ("23")

# Readings to take with a multimeter: the middle of the divider, which
# turns pin 18's resting 5 V into 3.3 V for the board, and the lamp's red.
bench.measure ("The board's GPIO47, the middle of the divider", red="d37", black="GND",
               expect="about 3.3 V", when="between messages")
bench.measure ("Pin 5, the lamp's red", red="5", black="GND", expect="about 5 V",
               when="after lamp on")
