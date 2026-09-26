# Lesson 41's screen, button, DHT11 and module A's divider stay where they
# were, with the Mega's 5V feeding the top rails at T+3; both LoRa
# modules, module B's divider and the power module go. The Meshtastic board
# stands below the board in module A's place, its USB socket to the left
# and its own cable powering it: its GPIO48 comes up into f53, where pin
# 19's wire still waits, its GPIO47 up beside the divider into c55, and
# its GND to the − rail past the divider. The RGB LED is the lamp, at its
# home beside the screen in columns 41-46, as in Lesson 15.
bench = Bench ("A Meshtastic board on pins 18 and 19, its GPIO47 through a 1 kΩ and 2 kΩ divider "
               "and powered from its own USB cable; the RGB LED lamp on pins 5 to 7, the DHT11 on "
               "pin 16, a button on 23 and the LCD on pins 31 to 36", columns=(1, 63))

bench.screen (text=("4f2a says:", "Hi Mega!"))

bench.module ("dht11", "dht", at=(8.58, -1.27))
bench.wire ("16", "dht.S")
bench.wire ("dht.+", "T+36")
bench.wire ("dht.−", "T-37")

bench.wire ("5", "j41", via=[(2.45, -1.55), (9.4, -1.55)])
bench.wire ("6", "j44", via=[(2.35, -1.65), (9.7, -1.65)])
bench.wire ("7", "j46", via=[(2.25, -1.75), (9.9, -1.75)])
bench.resistor ("220 Ω", "g41", "e41")
bench.resistor ("220 Ω", "g44", "e44")
bench.resistor ("220 Ω", "g46", "e46")
bench.rgb_led (red="a41", common="B-42", green="a44", blue="a46")

bench.module ("mesh_board", "node", at=(9.2, 3.75), facing="up")
bench.wire ("18", "j55", via=[(3.55, -2.35), (10.8, -2.35)])
bench.resistor ("1 kΩ", "g55", "e55")
bench.resistor ("2 kΩ", "a55", "B-55")
bench.wire ("node.47", "c55", color="white")
bench.wire ("node.48", "f53", color="grey")
bench.wire ("19", "j53", via=[(3.65, -2.25), (10.6, -2.25)])
bench.wire ("node.GND", "B-59")

bench.wire ("23", "j38", via=[(8.45, 0.95), (9.1, 0.95)])
bench.button (38)
bench.wire ("a40", "B-40")

bench.closeup (33, 63)

# Readings to take with a multimeter: the middle of the divider, which
# turns pin 18's resting 5 V into 3.3 V for the board, and the lamp's red.
bench.measure ("The board's GPIO47, the middle of the divider", red="d55", black="GND",
               expect="about 3.3 V", when="between messages")
bench.measure ("Pin 5, the lamp's red", red="5", black="GND", expect="about 5 V",
               when="after lamp on")
