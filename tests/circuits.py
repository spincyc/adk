"""The circuit model's own checks, on small made-up circuits.

    python3 tests/circuits.py

A circuit.py that could never work must stop the site, and tests/pins.py
must see what each pin does. These build circuits that break the rules one
at a time, and some that keep them, and say which failed.
"""

import os
import sys
import tempfile

ROOT = os.path.dirname (os.path.dirname (os.path.abspath (__file__)))
# Nothing may be written outside build/, so the theme's modules leave no
# byte-code in docs/_theme.
sys.dont_write_bytecode = True
sys.path.insert (0, os.path.join (ROOT, "docs", "_theme"))

from bench import Bench, hole_words, load, pin_words  # noqa: E402
from drawing import Drawing  # noqa: E402
from parts import bands_for  # noqa: E402
from route import Router, node  # noqa: E402

failures = []


# The red LED on pin 26, as Lesson 1 builds it.
def blink (bench, pin="26", hole="j6"):
    bench.wire (pin, hole)
    bench.resistor ("220 Ω", "g6", "e6")
    bench.led ("red", anode="b6", cathode="b7")
    bench.wire ("a7", "B-7")
    return bench


def button (bench, pin="22"):
    bench.wire (pin, "j2")
    bench.button (2)
    bench.wire ("a4", "B-4")
    return bench


def check (name, build, error=None, draw=False):
    try:
        result = build (Bench ("test", columns=(1, 20)))
        result.finish ()
        if draw:
            Drawing (result).svg ()
    except ValueError as raised:
        if error is None or error not in str (raised):
            failures.append (f"{name}: raised {raised}")
        return None
    if error:
        failures.append (f"{name}: should have raised {error!r}")
    return result


def expect (name, got, wanted):
    if got != wanted:
        failures.append (f"{name}: {got!r}, not {wanted!r}")


expect ("10 Ω five-band colors", bands_for ("10 Ω"),
        ["brown", "black", "black", "gold", "brown"])


# What finish () refuses.
check ("a good circuit", blink)
check ("a wire in the wrong hole", lambda b: blink (b, hole="j8"), "pin 26 reaches nothing")
check ("a pin on the + rail", lambda b: blink (b).wire ("27", "T+12"), "shorted to 5V")
check ("a pin on the − rail", lambda b: blink (b).wire ("27", "B-12"), "shorted to GND")
check ("two legs in one strip",
       lambda b: b.wire ("26", "j6").resistor ("220 Ω", "g6", "i6").led ("red", anode="b6",
                                                                          cathode="b7"),
       "share column 6 f-j")
check ("a button's joined legs", button)
capacitor = check ("a polarized capacitor", lambda b: b.capacitor ("1000 µF", "e6", "f6",
                                                                    polarized=True), draw=True)
expect ("capacitor polarity is in its build step", capacitor.items[0][2].places[1][1],
        "striped − leg")
reversed_capacitor = check ("a reversed horizontal capacitor",
                           lambda b: b.capacitor ("10 µF", "c16", "c15", polarized=True),
                           draw=True)
expect ("a reversed capacitor's left lead reaches its negative hole",
        reversed_capacitor.parts[0].shapes (reversed_capacitor)[1][1],
        reversed_capacitor.hole_xy ("c15")[0])
coil = check ("an inductor", lambda b: b.inductor ("100 mH", "e6", "f6"), draw=True)
expect ("inductor build step", coil.items[0][2].what, "100 mH inductor")
check ("a knob's legs in one row",
       lambda b: b.potentiometer ("e5", "e6", "e7"), "across the gap")
check ("a knob across the gap",
       lambda b: b.potentiometer ("f5", "d6", "f7").wire ("j5", "T-5").wire ("j7", "T+7")
       .wire ("A0", "a6"))

# What the drawing refuses.
check ("waypoints on one grid point",
       lambda b: blink (b, "26").wire ("27", "j12", via=[(5.6, 1.0), (5.62, 1.0)])
       .resistor ("220 Ω", "g12", "e12").led ("yellow", anode="b12", cathode="b13")
       .wire ("a13", "B-13"), "fall on one point", draw=True)
check ("a waypoint that turns back",
       lambda b: blink (b).wire ("27", "j12", via=[(5.6, 1.0), (5.9, 1.0), (5.7, 1.0)])
       .resistor ("220 Ω", "g12", "e12").led ("yellow", anode="b12", cathode="b13")
       .wire ("a13", "B-13"), "turns back", draw=True)

# What the pins check sees.
both = check ("an LED and a button", lambda b: button (blink (b)))
expect ("pin modes", {pin: mode for pin, (mode, _) in both.pin_modes ().items ()},
        {"26": "output", "22": "input"})
swapped = check ("an LED and a button swapped", lambda b: button (blink (b, pin="22"), pin="26"))
expect ("swapped pin modes", {pin: mode for pin, (mode, _) in swapped.pin_modes ().items ()},
        {"22": "output", "26": "input"})
expect ("SDA is pin 20", check ("SDA", lambda b: blink (b, pin="SDA")).signal_pins (), {"20"})

# Module pins by their other names, but never at the wrong voltage.
modem = Bench ("test", columns=(1, 20)).module ("lora_modem", "modem", at=(6.0, 3.45), facing="up")
expect ("a 3.3 V modem's VCC", modem.module_pin ("modem.VCC")[1].name, "VDD")
try:
    modem.module_pin ("modem.5V")
    failures.append ("a 3.3 V modem's 5V: found a pin")
except ValueError as error:
    expect ("a 3.3 V modem's 5V", str (error), "the LoRa modem's VDD takes 3.3V, not 5V")

# A two-board lesson's circuit.
TWO = '''
a = Bench ("Board A", columns=(1, 20), sketch="Blinker")
a.wire ("26", "j6").resistor ("220 Ω", "g6", "e6").led ("red", anode="b6", cathode="b7")
a.wire ("a7", "B-7")
b = Bench ("Board B", columns=(1, 20), sketch="Presser")
b.wire ("22", "j2").button (2).wire ("a4", "B-4")
boards = {BOARDS}
'''
for boards, error in (('{"A": a, "B": b}', None), ('{"B": b, "A": a}', "in order"),
                      ('{"A": a}', "in order")):
    with tempfile.NamedTemporaryFile ("w", suffix=".py", delete=False) as file:
        file.write (TWO.replace ("{BOARDS}", boards))
    try:
        loaded = load (file.name)
        if error:
            failures.append (f"boards {boards}: should have raised {error!r}")
        else:
            expect ("board letters", [bench.board for bench in loaded.values ()], ["A", "B"])
    except ValueError as raised:
        if not error or error not in str (raised):
            failures.append (f"boards {boards}: raised {raised}")
    finally:
        os.unlink (file.name)

# Parts at their homes: a home_* call builds just what the lesson wrote out
# by hand. Adding the screen must not move a recurring part or its wires.
def finished (bench):
    bench.finish ()
    return bench


by_hand = finished (button (blink (Bench ("test", columns=(1, 20)))))
at_home = finished (Bench ("test", columns=(1, 20)).home_led ("26", "red").home_button ("22"))
expect ("parts at their homes", at_home.used, by_hand.used)
expect ("their connections", at_home.connections (), by_hand.connections ())

beside = Bench ("test", columns=(1, 62)).screen ()
beside.home_button ("23").home_buzzer ("active").home_rgb_led (green_resistor=13).home_knob ()
beside = finished (beside)
for hole, what in (("j8", "the button on 23"), ("f33", "the buzzer"), ("a6", "the RGB LED"),
                   ("f39", "the knob"), ("a47", "the LCD's first pin"),
                   ("a62", "the LCD's last pin")):
    expect (f"{what} beside the screen, in {hole}", hole in beside.used, True)

# Compare the complete home, including its power jumpers, on both boards.
for home, args in (("home_button", ("23",)), ("home_rgb_led", ()),
                    ("home_buzzer", ("passive",)), ("home_buzzer", ("active",)),
                    ("home_rfid", ()), ("home_gy521", ()), ("home_knob", ()),
                    ("home_divider", ("photoresistor",)), ("home_encoder", ()),
                    ("home_modem", ()), ("home_servo", ())):
    homes = []
    for screen in (False, True):
        bench = Bench ("test", columns=(1, 63))
        if screen:
            bench.screen ()
        before, wires = set (bench.used), len (bench.wires)
        getattr (bench, home) (*args)
        homes.append ((set (bench.used) - before, bench.wires[wires:]))
    expect (f"{home} keeps its holes and wires with the screen", homes[1], homes[0])

plain = Bench ("test", columns=(1, 63)).power_module ()
bridged = Bench ("test", columns=(1, 63)).power_module ()
plain.home_servo ()
bridged.home_modem ().home_servo ()
expect ("the servo stays put beside the modem",
        bridged.modules["servo"].reach_box (), plain.modules["servo"].reach_box ())
finished (bridged)

# These combinations used to force parts to move or cover their power holes.
analog = Bench ("test", columns=(1, 63))
for pin in ("26", "27", "28", "29", "30"):
    analog.home_led (pin, "red")
finished (analog.home_buzzer ("passive").home_divider ("photoresistor").home_knob ())
tilting = finished (Bench ("test", columns=(1, 63)).home_encoder ().home_gy521 ())
expect ("the encoder's power holes stay clear of the accelerometer",
        {"T-18", "T+19"} & tilting._covered (), set ())
check ("a button covers the RGB LED's normal green resistor",
       lambda b: b.home_rgb_led ().home_button ("23"), "would cover e9")
rgb_button = finished (Bench ("test", columns=(1, 63)).home_rgb_led (green_resistor=13)
                       .home_button ("23"))
expect ("moving the green resistor preserves all three LED signals",
        {pin: mode for pin, (mode, _) in rgb_button.pin_modes ().items ()},
        {"5": "output", "6": "output", "7": "output", "23": "input"})
# The green jumper and button ground crowd all three adjacent G-label
# positions. The bench must still label that leg, using a leader, and the
# other three beside theirs.
legs = Drawing (rgb_button).svg ()
expect ("the RGB LED's legs are named on the bench",
        [f">{letter}</text>" in legs for letter in ("R", "−", "G", "B")], [True] * 4)

# The active buzzer's transistor, diode and resistors stand close together.
# Each is still named on the bench: beside itself, or further out on a leader.
crowded = Drawing (finished (Bench ("test", columns=(1, 63)).home_buzzer ("active"))).svg ()
for name in ("S8050 transistor", "1N4007 diode", "10 kΩ", "1 kΩ", "active buzzer"):
    expect (f"the {name} is named on the bench", f">{name}</text>" in crowded, True)

# Automatic links are left of the LCD's body and remain standard wires
# for the generated carry-over steps and the close-up's extent.
links = finished (Bench ("test", columns=(1, 63)).home_knob ())
ground_link = next (wire for wire in links.wires
                    if {end[1] for end in wire[:2]} == {"B-41", "T-41"})
expect ("the new rail link is standard", links.is_standard (ground_link), True)
bottom = finished (Bench ("test", columns=(1, 63)).home_beam ())
positive_link = next (wire for wire in bottom.wires
                      if {end[1] for end in wire[:2]} == {"T+42", "B+42"})
expect ("the new positive rail link is standard", bottom.is_standard (positive_link), True)
lcd = finished (Bench ("test", columns=(1, 63)).screen ())
expect ("the rail links and power feed stay clear of the LCD",
        {"B-41", "T-41", "B-42", "B+42", "T+42"} & lcd._covered (), set ())
separate = finished (Bench ("test", columns=(1, 63)).power_module ("3.3V").screen ()
                     .home_modem (power="B+29"))
expect ("the screen keeps the Mega's 5 V", separate._powered ("T+", "5V"), True)
expect ("the modem keeps the power module's 3.3 V", separate._powered ("B+", "3.3V"), True)
expect ("the two positive rails are separate", separate._powered ("B+", "5V"), False)

sensing = finished (Bench ("test", columns=(1, 50)).home_ultrasonic ().home_modem ())
expect ("the modem on Serial2 beside the ultrasonic sensor",
        {"14", "15", "16", "17"} <= sensing.signal_pins (), True)

# Electrical interfaces: GPIO only controls the active buzzer's base; it
# never supplies the buzzer. Reader inputs have dividers, and the two I2C
# voltage domains remain separate. These checks cannot validate real parts.
def net_of (bench, member):
    return next (net for net in bench.nets () if member in net)


active = finished (Bench ("test", columns=(1, 63)).home_buzzer ("active"))
expect ("the active buzzer takes rail power",
        "pin 5V" in net_of (active, "active buzzer: + leg (long)"), True)
expect ("the buzzer's negative goes to the collector",
        "S8050 transistor: C, collector" in net_of (active, "active buzzer: − leg"), True)
expect ("the emitter is grounded",
        "pin GND" in net_of (active, "S8050 transistor: E, emitter"), True)
expect ("pin 12 has only the current-limiting resistor",
        net_of (active, "pin 12") - {"pin 12", "column 32 a-e"},
        {"1 kΩ resistor: one end"})
expect ("the base has its resistor and startup pull-down",
        net_of (active, "S8050 transistor: B, base") - {"column 30 a-e"},
        {"S8050 transistor: B, base", "1 kΩ resistor: other end", "10 kΩ resistor: one end"})
expect ("the diode catches a positive collector spike",
        "1N4007 diode: anode, unbanded end" in net_of (active, "active buzzer: − leg")
        and "1N4007 diode: cathode, banded end" in net_of (active, "active buzzer: + leg (long)"),
        True)

reader = finished (Bench ("test", columns=(1, 63)).home_rfid ())
for pin, signal in (("53", "SDA"), ("52", "SCK"), ("51", "MOSI"), ("45", "RST")):
    output = net_of (reader, f"pin {pin}")
    input_net = net_of (reader, f"RFID reader: {signal}")
    series = next (member for member in output if member.startswith ("1 kΩ resistor"))
    shunt = next (member for member in input_net if member.startswith ("2 kΩ resistor"))
    expect (f"RFID {signal} has a series 1 kΩ",
            series.replace (": one end", ": other end") in input_net, True)
    expect (f"RFID {signal} has 2 kΩ to ground",
            "pin GND" in net_of (reader, shunt.replace (": one end", ": other end")), True)
    expect (f"RFID {signal} never gets direct 5 V", f"pin {pin}" in input_net, False)
expect ("RFID MISO goes directly back to the Mega",
        "pin 50" in net_of (reader, "RFID reader: MISO"), True)

gyro = finished (Bench ("test", columns=(1, 63)).home_gy521 ().home_modem (power="e5"))
for pin, channel, signal in (("20", "1", "SDA"), ("21", "2", "SCL")):
    high = net_of (gyro, f"I2C level shifter: B{channel}")
    low = net_of (gyro, f"I2C level shifter: A{channel}")
    expect (f"I2C {signal} translates to the sensor", f"pin {pin}" in high
            and f"GY-521: {signal}" in low and not high & low, True)
expect ("the I2C high supply is 5 V",
        "pin 5V" in net_of (gyro, "I2C level shifter: HV"), True)
expect ("the I2C low supply is 3.3 V",
        "pin 3.3V" in net_of (gyro, "I2C level shifter: LV"), True)
expect ("the modem can share the shifter's 3.3 V supply",
        "pin 3.3V" in net_of (gyro, "LoRa modem: VDD"), True)
left_of = lambda bench, a, b: bench.xy (("module", a))[0] < bench.xy (("module", b))[0]
expect ("the level shifter's B side faces the Mega, its A side the GY-521",
        left_of (gyro, "levels.B1", "levels.A1") and left_of (gyro, "levels.HV", "levels.LV"), True)

# Wire colors: black, red and orange for GND, 5 V and 3.3 V by what each
# wire carries, never for a signal; signals that end side by side, or
# cross, differ.
def color_of (bench, a, b):
    return next (color for start, end, color, _ in bench.wires if {start[1], end[1]} == {a, b})


expect ("the Mega's 3.3 V wire is orange", color_of (gyro, "3.3V", "a5"), "orange")
expect ("3.3 V through a strip is orange", color_of (gyro, "b5", "levels.LV"), "orange")
expect ("the modem's 3.3 V is orange", color_of (gyro, "e5", "modem.VDD"), "orange")
expect ("5 V from the + rail is red", color_of (gyro, "T+5", "levels.HV"), "red")
railed_modem = finished (Bench ("test", columns=(1, 63)).power_module ("3.3V")
                         .home_modem (power="B+29"))
expect ("a 3.3 V rail's wire is orange", color_of (railed_modem, "B+29", "modem.VDD"), "orange")
expect ("the power module's 3.3 V wire is orange",
        color_of (railed_modem, "power.3.3V", "B+42"), "orange")
check ("an orange signal", lambda b: blink (b).wire ("27", "j12", color="orange")
       .resistor ("220 Ω", "g12", "e12").led ("yellow", anode="b12", cathode="b13")
       .wire ("a13", "B-13"), "orange is kept for 3.3V")
check ("a red wire on GND", lambda b: blink (b).wire ("a10", "B-10", color="red"),
       "red is kept for 5V")
pins = [pin.name for pin in reader.modules["rfid"].pins ()]
for one, other in zip (pins, pins[1:]):
    colors = {color for start, end, color, _ in reader.wires
              for pin in (one, other) if f"rfid.{pin}" in (start[1], end[1])}
    if "IRQ" not in (one, other):
        expect (f"the RFID reader's {one} and {other} wires differ", len (colors), 2)
crossing = finished (Bench ("test", columns=(1, 40)).wire ("a4", "a32").wire ("a7", "b33"))
expect ("two jumpers that cross differ in color",
        color_of (crossing, "a4", "a32") != color_of (crossing, "a7", "b33"), True)

# A wire takes a costly short way rather than a cheap one three times as
# long, as round the far end of the Mega: here a strip that costs much to
# cross, with a way round it 100 units off.
router = Router ((-200, -200, 200, 200), (0, 0, 0, 0))
router.cost (("rect", -2, -98, 2, 98), 400)
path, _ = router.route ([node ((-50, 0)), node ((50, 0))])
expect ("a wire crosses rather than going far round", max (abs (j) for _, j in path) * 5 < 30, True)

# A trace for the scope: its tip moves off a hole a wire fills, its ground
# clip finds the − rail, and it goes to channel 1 or 2.
traced = finished (Bench ("test", columns=(1, 20)).module ("generator", "wave", at=(1, 4))
                   .wire ("wave.OUT", "j6").resistor ("1 kΩ", "g6", "e6").wire ("a6", "B-6")
                   .wire ("wave.GND", "B-5").probe ("CH1", tip="j6", ground="GND"))
(tip, _), (ground, words) = traced.probe_points (0)
expect ("a probe's tip beside the wire it was aimed at",
        tip != "j6" and traced.strip_of (tip) == traced.strip_of ("j6"), True)
expect ("a probe's ground clip on the − rail", ground.startswith ("B-") and "GND" in words, True)
expect ("the generator's own leads", traced.items[2][2].what, "lead")
check ("a third channel", lambda b: blink (b).probe ("CH3", tip="j6", channel=3), "channel 1 or 2")
door = finished (Bench ("test", columns=(1, 63)).power_module ("3.3V")
                 .home_buzzer ("active").home_rfid ().home_modem (power="B+29"))
expect ("the door's buzzer keeps 5 V beside its 3.3 V modem",
        door._powered ("T+", "5V") and door._powered ("B+", "3.3V")
        and not door._powered ("B+", "5V"), True)

# Both multiplexed-display lessons must limit current while a digit is
# actually on; a low average duty cycle cannot excuse excess peak current.
for lesson in ("011-four-digits", "012-stopwatch"):
    display = load (os.path.join (ROOT, "docs", "lessons", lesson, "circuit.py"))[""]
    segment_resistors = [part for part in display.parts if part.name == "2 kΩ resistor"]
    expect (f"{lesson} limits all eight segment currents", len (segment_resistors), 8)

# The build's stages: a home_* call is one, named for its part and the
# pins it wires; steps in no stage are named for their first part other
# than a resistor, and a module placed among them starts another; stage ()
# names the steps after it.
def titles (bench):
    stages = []
    for _, _, step in bench.items:
        if step.stage not in stages:
            stages.append (step.stage)
    return [bench.stage_title (stage) for stage in stages]


staged = Bench ("test", columns=(1, 20)).home_led ("26", "red")
staged.wire ("27", "j12").resistor ("220 Ω", "g12", "e12")
staged.led ("yellow", anode="b12", cathode="b13").wire ("a13", "B-13")
staged.module ("lora_modem", "modem", at=(6.0, 3.45), facing="up").wire ("modem.GND", "B-15")
staged.stage ("the link").wire ("T-18", "B-18")
expect ("stage titles", titles (finished (staged)),
        ["connect GND to the − rail", "the red LED on pin 26", "the yellow LED on pin 27",
         "the LoRa modem", "the link"])
powered = load (os.path.join (ROOT, "docs", "lessons", "007-dimmer", "circuit.py"))[""]
expect ("each automatic rail connection has an accurate title", titles (powered)[:3],
        ["connect GND to the − rail", "connect 5 V to the + rail", "join the − rails"])
expect ("pins in words", pin_words ({"36", "A0", "9", "31", "33", "10", "32", "34", "35"}),
        "pins 9, 10, 31–36 and A0")
expect ("a pin in words", pin_words ({"A3"}), "pin A3")

# A rail's hole in words, never as B-7, whose B reads as row b: in running
# text, and in the steps table as the rail with its column beneath.
expect ("a rail's hole in words", hole_words ("B-7"), "the bottom − rail by column 7")
expect ("a hole in words", hole_words ("j6"), "j6")
railed = finished (blink (Bench ("test", columns=(1, 20))))
expect ("a rail in the steps", railed.items[-1][2].places,
        (("a7", ""), ("bottom − rail", "by column 7")))

if failures:
    sys.exit ("tests/circuits.py:\n  " + "\n  ".join (failures))
