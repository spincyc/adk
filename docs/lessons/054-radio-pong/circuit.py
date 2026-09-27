# Two boards built the same: Lesson 27's game on each, and the LoRa modem
# at the bridge's home. The LED matrix lies below the gap between the Mega
# and the breadboard, input pins up, its GND from B-5 and VCC from the
# inner 5V pin; the joystick lies below the Mega under A3 and A4, powered
# from the power header's 5V and the inner GND pin, its switch on 22; the
# passive buzzer stands across the gap in column 34, pin 10 into j34 and
# its 220 Ω from a34 to the − rail.
#
# The modem lies below the board under columns 42-47, aerial down, its
# TXD up into f44 beside RX3 (pin 15) in j44, TX3 (pin 14) into j46 and
# down through 1 kΩ and 2 kΩ to the − rail, the modem's RXD into c46
# between them, its GND into B-42 and its VDD from the Mega's 3.3V pin.
# Pins 10, 14 and 15 cross over the top in lanes, 10 highest, so its wire
# crosses the other two just once each, over the board's end.

# The matrix is drawn turned half round, so its picture is given upside
# down: the paddle on the bottom row, the ball on its way up.
COURT = ["........",
         "........",
         "........",
         ".....#..",
         "........",
         "........",
         "........",
         "..###..."]


def player (title, sketch):
    bench = Bench (title, columns=(1, 50), sketch=sketch)

    bench.home_modem (tx=[(3.15, 0.35), (9.9, 0.35)], rx=[(3.25, 0.45), (9.7, 0.45)])

    bench.home_matrix (pixels=[row[::-1] for row in reversed (COURT)])

    bench.home_joystick ()

    bench.home_buzzer ("passive", via=[(1.89, 0.25), (8.7, 0.25)])
    bench.closeup (1, 50)

    # Readings to take with a multimeter, before anyone serves: TX3
    # resting at 5 V, and the modem's RXD, where the divider makes it 3.3 V.
    bench.measure ("TX3, pin 14, resting", red="14", black="GND", expect="about 5 V",
                   when="no ball in play")
    bench.measure ("The modem's RXD, resting", red="d46", black="GND", expect="about 3.3 V",
                   when="no ball in play")
    return bench


boards = {"A": player ("Board A: an LED matrix on pins 47 to 49, a joystick on A3 and A4 with its "
                       "switch on 22, a passive buzzer on pin 10 through 220 Ω, and a LoRa modem "
                       "on pins 14 and 15, its VDD from the Mega's 3.3V pin", "Ping"),
          "B": player ("Board B: built as Board A", "Pong")}
