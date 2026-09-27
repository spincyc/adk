# Two boards built the same way, each on a Mega of its own. The button on
# 22 and the LEDs on 26 (red) and 27 (yellow) stand at their homes in
# columns 2-4, 6 and 12, their wires nested as in Lesson 2. The LoRa modem
# lies at its bridge home below the board, aerial down, under columns
# 42-47, where it stays in every lesson of the two-board arcs: its TXD up
# into f44 beside the wire from RX3 (pin 15) in j44; TX3 (pin 14) into
# j46, through 1 kΩ across the gap and 2 kΩ down to the − rail, and its
# RXD into c46, the divider's middle; its GND into B-42. Its VDD takes the
# Mega's own 3.3V pin, straight from the power header: at 10 dBm the modem
# stays within what that pin can give, so the board needs no power module.
def board (letter, sketch):
    bench = Bench (f"Board {letter}: a button on pin 22, a red LED on 26, a yellow LED on 27, and "
                   f"a LoRa modem on Serial3 (pins 14 and 15), its RXD through a 1 kΩ and 2 kΩ "
                   f"divider and its VDD on the Mega's 3.3V pin", columns=(1, 50), sketch=sketch)

    bench.home_button ("22")

    bench.home_led ("26", "red", via=[(4.45, 1.0), (4.45, 1.15)])

    bench.home_led ("27", "yellow", via=[(4.35, 1.05), (4.35, 1.25)])

    bench.home_modem ()
    return bench


boards = {"A": board ("A", "BoardA"), "B": board ("B", "BoardB")}

# Readings to take with a multimeter on Board A (Board B reads the same):
# pin 14, TX3, resting at 5 V between messages, and the modem's RXD, the
# middle of its divider, at two thirds of that.
boards["A"].measure ("Pin 14, TX3, resting", red="14", black="GND", expect="about 5 V",
                     when="between messages")
boards["A"].measure ("The modem's RXD, the middle of its divider", red="d46", black="GND",
                     expect="about 3.3 V", when="between messages")
