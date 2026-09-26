"""Hold each sketch to its circuit.

    python3 tests/pins.py Lesson03ReactionDuel claimed.txt
    python3 tests/pins.py Lesson44RemoteDial/Dial claimed.txt

claimed.txt lists each pin the sketch's parts claimed when its setup () ran
on the host (tests/probe/pins.cpp), and the mode it was left in, output or
input. The circuit is the lesson's circuit.py, or in a two-board lesson the
board whose sketch this is. Two things must hold:

- The pins claimed are exactly the Mega pins the circuit wires, but for the
  built-in LED on pin 13, which needs no wire.
- Where the circuit shows what a pin does, driving an LED or a module's
  input, or reading a button, a knob or a sensor, the sketch claims it the
  same way: as an output or an input.

So a wire the sketch doesn't use, a pin the build leaves unwired, or an LED
swapped with a button fails. Two pins wired to parts of one kind, such as
two LEDs, can still be swapped unnoticed.
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname (os.path.dirname (os.path.abspath (__file__)))
sys.path.insert (0, os.path.join (ROOT, "docs", "_theme"))

import bench  # noqa: E402


# The circuit an example's sketch runs on.
def circuit (example):
    folder, _, sketch = example.partition ("/")
    number = re.match (r"Lesson(\d\d)", folder).group (1)
    paths = glob.glob (os.path.join (ROOT, "docs", "lessons", f"{number}-*", "circuit.py"))
    if len (paths) != 1:
        sys.exit (f"{example}: expected one docs/lessons/{number}-*/circuit.py")
    path = os.path.relpath (paths[0], ROOT)
    expected = bench.example (os.path.basename (os.path.dirname (path)))
    if folder != expected:
        sys.exit (f"{example}: Lesson {number}'s example is examples/{expected}")
    try:
        boards = bench.load (os.path.join (ROOT, path))
    except ValueError as error:
        sys.exit (f"{path}: {error}")
    found = [board for board in boards.values () if (board.sketch or "") == sketch]
    if not found:
        sketches = " and ".join (f"examples/{folder}/{board.sketch}" for board in boards.values ())
        sys.exit (f"{example}: {path} has no board for this sketch; "
                  + (f"its boards' sketches are {sketches}" if "" not in boards else
                     f"its one sketch is examples/{folder}/{folder}.ino"))
    return found[0]


def main (example, claimed_path):
    claimed = dict (line.split () for line in open (claimed_path, encoding="utf-8"))
    board = circuit (example)
    wired = board.signal_pins ()
    missing = set (claimed) - wired - {"13"}
    extra = wired - set (claimed)
    if missing or extra:
        sys.exit (f"{example}: the sketch and its circuit disagree. "
                  f"Claimed but not wired: {sorted (missing) or 'none'}. "
                  f"Wired but not claimed: {sorted (extra) or 'none'}.")
    wrong = [f"pin {pin} {'drives' if mode == 'output' else 'reads'} {' and '.join (things)}, "
             f"but the sketch claims it as an {claimed[pin]}"
             for pin, (mode, things) in sorted (board.pin_modes ().items ())
             if claimed[pin] != mode]
    if wrong:
        sys.exit (f"{example}: " + "; ".join (wrong) + ".")


if __name__ == "__main__":
    main (*sys.argv[1:])
