# Each board keeps its LoRa modem at its bridge home from Lesson 49: below
# the board under columns 42-47, TX3 (pin 14) through the divider in
# column 46, RX3 (pin 15) in j44, VDD on the Mega's 3.3V pin.
#
# Board A, the tilt: the keypad and the screen come out. The GY-521 stands
# in row j, columns 9 to 16, as in Lesson 28: a red jumper from T+7 feeds
# VCC, two black jumpers carry GND across the gap to the bottom − rail, and
# pins 20 and 21 come over the top header and down onto the board beside
# it. The Mega's built-in L LED shows the link, so it needs no wire.
tilt = Bench ("Board A: a GY-521 accelerometer on the I2C pins 20 and 21, and the LoRa modem "
              "on Serial3 (pins 14 and 15)", columns=(1, 50), sketch="Tilt")

tilt.header_module ("gy521", first=9, row="j")
tilt.wire ("T+7", "i9")
tilt.wire ("f10", "e10", color="black")
tilt.wire ("a10", "B-10")
tilt.wire ("21", "g11", via=[(3.85, 0.55), (5.75, 0.55), (5.75, 1.35)])
tilt.wire ("20", "h12", via=[(3.75, 0.5), (5.85, 0.5), (5.85, 1.25)])

tilt.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
tilt.wire ("modem.GND", "B-42")
tilt.wire ("modem.VDD", "3.3V")
tilt.wire ("14", "j46", via=[(3.15, -0.35), (9.9, -0.35)])
tilt.resistor ("1 kΩ", "g46", "e46")
tilt.resistor ("2 kΩ", "a46", "B-46")
tilt.wire ("modem.RXD", "c46", color="grey")
tilt.wire ("modem.TXD", "f44", color="purple")
tilt.wire ("15", "j44", via=[(3.25, -0.25), (9.7, -0.25)])

# Board B, the ball: the RGB LED and its resistors come out; the power
# module, the servo and the modem stay. The LED matrix lies at its home
# below the gap between the Mega and the breadboard, as in Lessons 25 to
# 30: VCC from the inner 5V pin at the top of the long header, GND into
# B-5. Pin 44's wire now drops beside the board, clear of the matrix, and
# runs down the modem's left side and under it to the servo.
BALL = ["........",
        "........",
        "........",
        "....#...",
        "........",
        "........",
        "........",
        "........"]

ball = Bench ("Board B: the LED matrix on pins 47 to 49, a servo on 44 powered by the power "
              "module, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 63),
              sketch="Ball")

ball.power_module ("right", top="off", bottom="5V")

# The matrix is drawn turned half round, so its picture is given upside down.
ball.module ("matrix", at=(4.22, 3.9), facing="up",
             pixels=[row[::-1] for row in reversed (BALL)])
ball.wire ("48", "matrix.CLK")
ball.wire ("49", "matrix.CS")
ball.wire ("47", "matrix.DIN")
ball.wire ("B-5", "matrix.GND")
ball.wire ("5V.long", "matrix.VCC")

ball.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
ball.wire ("modem.GND", "B-42")
ball.wire ("modem.VDD", "3.3V")
ball.wire ("14", "j46")
ball.resistor ("1 kΩ", "g46", "e46")
ball.resistor ("2 kΩ", "a46", "B-46")
ball.wire ("modem.RXD", "c46", color="grey")
ball.wire ("modem.TXD", "f44", color="purple")
ball.wire ("15", "j44")

ball.module ("servo", "servo", at=(9.85, 5.7), facing="up")
ball.wire ("servo.+", "B+53")
ball.wire ("servo.−", "B-54")
ball.wire ("44", "servo.signal", via=[(4.85, 1.95), (4.85, 2.95), (9.3, 2.95), (9.3, 5.55),
                                      (10.5, 5.55)])

ball.closeup (38, 63)

boards = {"A": tilt, "B": ball}
