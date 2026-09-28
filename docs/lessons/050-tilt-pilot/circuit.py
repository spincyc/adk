# Each board keeps its LoRa modem at its bridge home from Lesson 49, on
# Serial3 (pins 14 and 15), VDD on the Mega's 3.3V pin.
#
# Board A, the tilt: the keypad and the screen come out. The GY-521 stands
# at its home on pins 20 and 21, as in Lesson 28. The Mega's built-in L
# LED shows the link, so it needs no wire.
tilt = Bench ("Board A: a GY-521 accelerometer on the I2C pins 20 and 21, and the LoRa modem "
              "on Serial3 (pins 14 and 15)", columns=(1, 50), sketch="Tilt")

tilt.home_gy521 ()

tilt.home_modem ()

# Board B, the ball: the RGB LED and its resistors come out; the power
# module, the servo and the modem stay. The LED matrix lies at its home,
# as in Lessons 25 to 30. The servo stays below the modem.
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

ball.power_module ()

# The matrix is drawn turned half round, so its picture is given upside down.
ball.home_matrix (pixels=[row[::-1] for row in reversed (BALL)])

ball.home_modem ()

ball.home_servo ()

ball.closeup (38, 63)

boards = {"A": tilt, "B": ball}
