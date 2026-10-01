# Each board keeps its LoRa modem at its bridge home from Lesson 49, on
# Serial3 (pins 14 and 15), powered by the Mega's 3.3V pin. Board A
# shares that supply with the level shifter through column 5.
#
# Board A, the tilt: the keypad and the screen come out. The GY-521 stands
# at its home on pins 20 and 21, as in Lesson 28. The Mega's built-in L
# LED shows the link, so it needs no wire.
tilt = Bench ("Board A: a GY-521 accelerometer on the I2C pins 20 and 21, and the LoRa modem "
              "on Serial3 (pins 14 and 15)", columns=(1, 50), sketch="Tilt")

tilt.home_gy521 ()

tilt.home_modem (power="e5")

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

boards = {"A": tilt, "B": ball}

# Readings to take with a multimeter on Board A: the Mega's 3.3V feed,
# which the modem and the level shifter's low side share, and the top +
# rail at 5 V, which feeds its high side.
tilt.measure ("The 3.3 V feed, shared by the modem and LV", red="3.3V", black="GND",
              expect="about 3.3 V", when="both boards running")
tilt.measure ("The top + rail, for HV and the GY-521", red="5V", black="GND",
              expect="about 5 V", when="both boards running")
