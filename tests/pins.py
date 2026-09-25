"""Hold each sketch to its lesson's circuit.

    python3 tests/pins.py Lesson03ReactionDuel claimed.txt

claimed.txt lists the pins the sketch's parts claimed when its setup () ran
on the host (tests/probe/pins.cpp). They must be exactly the Mega pins the
lesson's circuit.py wires, so the drawings, build steps and sketch can never
disagree about a pin.
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname (os.path.dirname (os.path.abspath (__file__)))
sys.path.insert (0, os.path.join (ROOT, "docs", "_theme"))

import bench  # noqa: E402


def circuit (example):
    number = re.match (r"Lesson(\d\d)", example).group (1)
    paths = glob.glob (os.path.join (ROOT, "docs", "lessons", f"{number}-*", "circuit.py"))
    if len (paths) != 1:
        sys.exit (f"{example}: expected one docs/lessons/{number}-*/circuit.py")
    scope = {name: getattr (bench, name) for name in dir (bench) if not name.startswith ("_")}
    exec (compile (open (paths[0], encoding="utf-8").read (), paths[0], "exec"), scope)
    return scope["bench"]


def main (example, claimed_path):
    claimed = set (open (claimed_path, encoding="utf-8").read ().split ())
    wired = circuit (example).signal_pins ()
    # The built-in LED is on the board itself and needs no wire.
    missing = claimed - wired - {"13"}
    extra = wired - claimed
    if missing or extra:
        sys.exit (f"{example}: the sketch and its circuit disagree. "
                  f"Claimed but not wired: {sorted (missing) or 'none'}. "
                  f"Wired but not claimed: {sorted (extra) or 'none'}.")


if __name__ == "__main__":
    main (*sys.argv[1:])
