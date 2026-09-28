# Handoff: the screen at the far end of the breadboard

Status on 2026-09-28: **a prototype and a plan awaiting the owner's review.**
This branch, `screen-at-far-end`, is local and not pushed. `main` is untouched
at `2b4f145`. The prototype breaks 25 of 58 board drawings, so it must not
be merged as it is.

## How this started

The owner asked why Lesson 55's rotary encoder sits above the Mega on
jumper wires when the breadboard "looks like it has plenty of room".

### Why the encoder isn't on the board in Lesson 55

The layout code doesn't think the whole board is full. It only checks the
encoder's one breadboard home, and that home is blocked. The encoder's home
is row a, columns 45–49. Its fallback, in `docs/kit.md`, is "Rotary encoder,
where the board is full: high above the Mega". The code comment in
`home_encoder` uses the same words. Both overstate the case: only the home
is blocked. Measured on Lesson 55's board:

| Columns | Why the encoder can't stand there |
|---|---|
| 25–37 | Look empty, but the LCD's board hangs off the bottom edge from column 6 to 37 (pins in a9–a24, 3 columns of board before pin 1, 13 after pin 16), covering row a and the bottom rails. |
| 39–42 | The 433 MHz receiver's board lies over the top rails from 40 to 52, where the encoder's power jumpers must go. |
| 38 and 43 | A rail gap: rail holes come in fives, with gaps at columns 8, 14, 20, 26, 32, 38, 44, 50, 56. |
| 44 onward, the home | The receiver's wires come up from below the board at columns 48–49, across the encoder's board. |

Columns 25–47 are empty in both halves, 23 columns. But everything in
25–37 is under the LCD, which is why the board looks emptier than it is.

### The smaller fix, verified

Keep the screen where it is, and stand the encoder in row a just right of
the LCD, with all five of its wires from the Mega (the pins the
above-the-Mega version already uses) instead of rail jumpers:

```python
bench.header_module ("encoder", pins=("GND", "+", "SW", "DT", "CLK"), first=39, row="a")
bench.wire ("GND.top", "e39")
bench.wire ("5V.long", "e40")
bench.wire ("22", "e41")
bench.wire ("19", "e42")
bench.wire ("18", "e43")
```

Tried on each lesson that puts the encoder above the Mega, at `first` 39–42:

| Lesson | Result |
|---|---|
| 55 | Fits at 39 and 40. At 41–42 a multimeter probe (B-46) falls under it, which moving the probe would fix. |
| 37 | Fits at 39. |
| 33, 39 | Blocked: the button on 23 beside the screen, in column 38, covers e39, and e40 holds a wire. It would need a small shuffle. |
| 44 (Board A) | Blocked at every column: the bridge modem's GND (B-42) is under the encoder's board. |

This was the recommendation, and it's still the cheaper path if the move
below is dropped.

## The owner's direction: move the screen

The owner proposed moving the LCD to the far right, "so only the pins are
on the board and the rest of the LCD is hanging off", and asked to make that
its canonical home across all lessons, to free up board space.

**The concern raised, and the decision.** The LCD already has only its pins
in the board: its body hangs off the bottom edge. The problem is its width.
At the far right about 13 of its 32 columns would overhang the end, which
gains space, but the right end is the busiest part of the course's layout.
23 of the 27 screen boards use columns 44–63 there:

- 015, 017, 018, 024, 029, 033, 036, 037, 038, 039, 040, 041, 042, 055
- 044A, 045A, 046B, 047B, 048B, 049A, 051B, 052A, 053A

Only 013, 014, 016 and 032 are clear. Moving the screen means new homes for
the power module (7 of those boards), the bridge's LoRa modem (11), the
radios, the servo, and the parts that sat beside the screen. On 2026-09-28
the owner, told this, chose to move the screen anyway, with a plan of the new
homes to be shown before re-laying out the boards. This document is that
plan, with the prototype behind it.

## The prototype on this branch

### What changed

In `docs/_theme/bench.py`:

- `screen ()`: `first` is 43, so the contrast knob stands across the gap in
  43–45 and the LCD's pins in a47–a62, laid out exactly as before, 38 columns
  further on. It widens the drawing to column 63.
- `_rail ()`: past a rail's last hole (61) it searches back, instead of
  looping forever.
- `home_button`, `home_buzzer`, `home_rgb_led`: no second homes beside the
  screen. Button 23, the buzzer and the RGB LED use their own homes (8–10,
  34, 6–11), which the screen no longer covers.
- `home_knob`: beside the screen, columns 39–41.
- `home_encoder`: beside the screen, on the board in row a, columns 15–19,
  with its usual rail jumpers. The circuits of 033, 037, 039, 044 and 055
  now call `home_encoder ()` rather than `home_encoder (above=True)`.
- `home_divider`: beside the screen, column 39 (40 elsewhere).
- `home_fm_radio`, `home_rf_receiver`, `home_rf_transmitter`: 18 columns
  nearer the Mega (j27–34, j30–33, j38–41). Their hand-set waypoints are
  gone, so the router finds its own way.
- `home_modem`: 18 columns nearer the Mega on every bridge board, below
  columns 24–29 (divider in column 28, TXD in f26, GND by column 24), so
  consecutive bridge lessons still keep it where it was.
- `home_servo`: plug under columns 34–36 (+ by 35, − by 36). It no longer
  lies lower beside the modem, which is now a conflict (below).
- `home_rtc`: the clock's rail wires by columns 13 and 15, not 29 and 30,
  clear of the FM radio.
- `power_module ()`: still to the right of the board, lifted 0.3 inch clear
  of the LCD's overhang, its wires into the bottom rails by column 42 in
  every lesson.
- `_power ()`: on a screen board, the rail links go by column 42, not 60/61.

The code comments and docstrings still describe the old columns: the
prototype was for finding out, not for keeping.

### What draws

`python3 trial.py` (below) on this branch: **33 of 58 board slots draw.** A
lesson that fails before its boards load counts once. The rest fail for
these reasons:

| Kind | Lessons | What it needs |
|---|---|---|
| Multimeter probe on a hole that moved or is covered | 013, 014, 016, 017, 018, 033, 036, 038, 043, 052, 054, 055 | New probe holes in each `circuit.py` |
| Loose parts where the screen now is, or on the power module's new holes | 040 and 041 (B±42), 042 (the Meshtastic board in a55), 046 (the RGB LED on e48) | Per-lesson edits: 046 can use `home_rgb_led ()` |
| Servo overlaps the moved modem | 044, 045, 049, 050, 051 | The servo lower on bridge boards again, or elsewhere |
| Wires the router can't get through | 047B and 048B (the modem's TXD to f26), 048A (pin 15 to j26); 017 (pin 44 to the servo), seen once its probes are removed | Waypoints, or small moves |
| A label with no room | 015 (the RGB LED's R); 049A (pin 27), seen before the servo moved and now hidden behind the servo overlap | Small moves |

Lesson 55 draws cleanly with the encoder on the board. The screen's six
signal wires arc over the whole board to reach it, and a few wires take long
detours that would want hand routing.

## Proposed homes

| Part | Now | Proposed |
|---|---|---|
| Screen | Knob 5–7, LCD a9–a24, board under 6–37 | Knob 43–45, LCD a47–a62, board off the bottom edge and past the end |
| Button 23, buzzer, RGB LED on screen boards | 38–40, 51, 41–46 | Their own homes: 8–10, 34, 6–11 |
| Knob (A0) on screen boards | 45–47 or 57–59 | 39–41 |
| Rotary encoder on screen boards | Above the Mega (5 lessons), on the board (Lesson 29) | On the board, row a, 15–19, in all 6 |
| Light or temperature divider on screen boards | 40 | 39 |
| FM radio | j45–52 | j27–34 |
| 433 MHz receiver / transmitter | j48–51 / j56–59 | j30–33 / j38–41 |
| Bridge LoRa modem (every bridge board) | Below 42–47 | Below 24–29 |
| Lessons 40–42's paired modems and Meshtastic board | Below 42–57 | Moved left to match, per lesson |
| Servo | Plug under 52–54 | Plug under 34–36; lower on bridge boards |
| Power module (every lesson) | Right of the board, into the bottom rails by column 61 | Right of the board, lifted, into the bottom rails by column 42 |
| Clock module's rail wires | 29 and 30 | 13 and 15 |
| Rail links on screen boards | 60 and 61 | 42 |

## Open questions for the owner

1. **The power module.** Its wires reach the bottom rails by column 42
   across the right end of the board, which the LCD's jumpers and signal
   wires now crowd. No drawing of it has been seen yet: Lesson 17, the first
   to try, failed on its servo wire. The alternatives, below the board at the
   left end or below the Mega, compete with the modems, the servo, the
   encoder's board and the stepper driver.
2. **The bridge modem's move** reaches every bridge board, screen or not, so
   that consecutive lessons keep it. The alternative, a second home on screen
   boards only, rebuilds it whenever a board gains or loses the screen.
3. **The screen's signal wires** from pins 31–36 now cross the whole board.
   Are six long wires over everything acceptable for a beginner?
4. **Or drop the move** and take the smaller fix above.

## The rest of the work, once the plan is approved

- Fix the failures in the table above, and look at every board's drawing.
- Rewrite the code comments and docstrings of every home that moved, and
  `docs/kit.md`'s homes tables, whose "beside the screen" column mostly goes
  away.
- Rewrite the hand-written text about positions in about 30 lesson pages:
  columns, "beside the screen", "above the Mega", the power module's
  "by column 61", the modem's "under columns 42–47". The build steps,
  "Keep from…" lines and connection lists regenerate on their own.
- Recount each lesson's parts-list wire counts: `make site` checks them.
- `make check`, a changelog entry, and commits in a few coherent pieces.

## Reproducing

Everything the prototype says can be checked by loading and drawing every
lesson:

```python
# trial.py: load and draw every lesson; report which boards fail, and why.
import concurrent.futures, glob, sys
sys.path.insert (0, "docs/_theme")

def trial (path):
    from bench import load
    from drawing import Drawing
    slug = path.split ("/")[-2]
    try:
        boards = load (path)
    except Exception as error:
        return [(slug, "", f"load: {error}")]
    out = []
    for letter, bench in boards.items ():
        try:
            Drawing (bench).page (letter)
            out.append ((slug, letter, "ok"))
        except Exception as error:
            out.append ((slug, letter, str (error)))
    return out

if __name__ == "__main__":
    paths = sorted (glob.glob ("docs/lessons/*/circuit.py"))
    with concurrent.futures.ProcessPoolExecutor () as pool:
        results = [r for rs in pool.map (trial, paths) for r in rs]
    for slug, letter, what in results:
        if what != "ok":
            print (f"{slug} {letter}: {what[:150]}")
    print (sum (what == "ok" for *_, what in results), "of", len (results), "boards draw")
```

Run it from the top of the repository with the system Python, which has
what `bench.py` needs. To see one board, draw it with
`Drawing (bench).page (letter)`: the first of the three results is the
whole bench as SVG, and a headless Chromium screenshot of an HTML page
holding it gives a PNG. That is how the renders described above were made.
