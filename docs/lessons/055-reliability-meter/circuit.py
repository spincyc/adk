# Two boards carried on from Lesson 54, each with its LoRa modem at the
# bridge's home on Serial3 (pins 14 and 15), its VDD from the Mega's 3.3V
# pin, and the LED matrix at its home below the gap beside the Mega.
#
# Board A, the meter, keeps all of Lesson 54's game: the joystick on A3
# and A4 with its switch on 22, and the passive buzzer on pin 10. The
# screen comes back to its home, as in Lesson 53, its contrast knob in
# columns 43 to 45 and its pins in a47 to a62, clear of everything else.
#
# Board B, the echo, keeps only the modem and the matrix: its joystick,
# buzzer and the buzzer's resistor come out.

# The matrices are drawn turned half round, so their pictures are given
# upside down. Each dot is one message of a test, read like a page: 1 to 8
# along the top row, 57 to 64 along the bottom. Message 20 never reached
# Board B, nor 42; 41 reached it, but its echo was lost on the way back.
METER = ["########",
         "########",
         "###.####",
         "########",
         "########",
         "..######",
         "########",
         "########"]

ECHO = ["########",
        "########",
        "###.####",
        "########",
        "########",
        "#.######",
        "########",
        "########"]


def upside_down (picture):
    return [row[::-1] for row in reversed (picture)]


meter = Bench ("Board A, the meter: the LCD on pins 31 to 36, an LED matrix on pins 47 to 49, a "
               "joystick on A3 and A4 with its switch on 22, a passive buzzer on pin 10 through "
               "220 Ω, and a LoRa modem on pins 14 and 15, its VDD from the Mega's 3.3V pin",
               columns=(1, 50), sketch="Meter")

meter.home_modem ()

meter.home_matrix (pixels=upside_down (METER))

meter.home_joystick ()

meter.home_buzzer ("passive")

meter.screen (text=("Quick 20 letters", "95% back 112ms"))
meter.closeup (1, 63)

# Readings to take with a multimeter on Board A between tests, while the
# serial lines to the modem rest: the modem's RXD, made 3.3 V from pin
# 14's 5 V by the divider, and its TXD, the modem's own 3.3 V, which the
# Mega reads straight on pin 15.
meter.measure ("The modem's RXD, resting", red="d28", black="GND", expect="about 3.3 V",
               when="between tests")
meter.measure ("The modem's TXD, resting", red="15", black="GND", expect="about 3.3 V",
               when="between tests")

echo = Bench ("Board B, the echo: an LED matrix on pins 47 to 49, and a LoRa modem on pins 14 "
              "and 15, its VDD from the Mega's 3.3V pin", columns=(1, 50), sketch="Echo")

echo.home_modem ()

echo.home_matrix (pixels=upside_down (ECHO))
echo.closeup (1, 50)

boards = {"A": meter, "B": echo}
