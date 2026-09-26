---
lesson: 47
promise: Guard a door with five tripwires and hear about it in another room, where the remote arms and disarms the alarm.
time: 90 minutes
level: 3
parts:
  - "Board A, by the door: Lesson 46's Board A, with its LoRa modem, divider and green LED"
  - "Board A: the HC-SR501 PIR sensor and the tilt switch, and the beam-break, obstacle and tap sensors (37 in 1 kit), from Lessons 23 and 35"
  - "Board A: a red LED, a 220 Ω resistor, 5 jumper wires and 12 female-to-male jumper wires"
  - "Board B, in the den: Lesson 46's Board B, with its LoRa modem, screen and clock module"
  - "Board B: the IR receiver and the kit's remote, the active buzzer, 2 jumper wires and 3 female-to-male jumper wires"
ideas:
  - An event sent as a count
  - Why the first number heard isn't news
  - A setting sent back the other way
  - Tripwires far from the alarm
---

## What you'll build

<!-- closeup B -->

Lesson 24's room alarm, split in two. Board A stands by the door with five
tripwires: the PIR, the tilt switch, the beam-break and obstacle sensors
from Lesson 23, and the tap sensor from Lesson 35. Board B sits in the den,
where you are. Whenever a tripwire goes off, the den's screen names it and
the time: `Knock   21:07:43`. Press POWER on the remote and the den arms
the alarm; the door's red LED lights to show it, and now a tripwire sets the
siren going until you press POWER again.

## The idea

**An event crosses as a count.** A tripwire going off is over in an
instant: `activated ()` is true for just one pass of `loop ()`. The bridge
keeps numbers the same on both boards, but a number that is 1 for an
instant might never be sent, since the bridge sends at most ten times a
second, or its message might be lost. So Board A **counts**: each time the
PIR goes off, `moves` goes up by one, and the bridge carries the count.

<p class="formula">motion=6 → motion=7: the PIR went off</p>

When the den sees a count go up, something happened at the door. If a
message is lost, the next one still carries the higher count, so nothing is
missed; if the PIR goes off twice before a message goes, the count jumps by
two, and the den still sees it change.

**The first number isn't news.** When the den starts, or first hears the
door, it learns counts like `motion=7`. That says where the count stands,
not that anything is happening now. So the den remembers the latest count
it heard from each tripwire, starting at −1, which means "not heard yet",
and only a count higher than the one it remembers is a trip. A count that
falls isn't one either: the door board has started again, from 0.

**Both ways.** A bridge carries names each way. The door shares five
counts; the den shares one number back, `armed`, 1 or 0, and the door
lights its red LED from it. A `bool` crosses as a whole number: `true` is
1 and `false` is 0.

**Tripwires far from the alarm.** The sensors are Lesson 23's, wired as
there where they can be. Two can't go to their homes. The obstacle sensor's
home below the board is the modem's now, and the tap sensor's home would
share the power header's one 5V pin with the PIR. So they take the places
Lesson 46's DHT11 and 18B20 had above the board, and their pins, 16 and
17. An `adk::Switch` works on any pin.

!!! question "Predict"
    Once it works, you'll arm the alarm and then unplug Board A, as a
    burglar might. Will the siren sound? What will the den's screen say?
    Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug both boards before you wire. Match each module's pins by the
    names printed on it, not by where they sit, as in Lesson 23. The
    obstacle sensor's fourth pin, **EN**, stays unconnected. Neither board
    has a motor or a servo, so neither needs the power module.

### Board A, by the door

Keep the modem, its divider and the green LED from Lesson 46, and take out
the four sensors and their wires. The red LED goes at its home in column
6. The PIR sits below the Mega and the beam-break sensor below the board
at their homes, and the tilt switch stands in c32 and c33, as in Lesson 23.
The obstacle sensor takes the DHT11's place above the board, powered from
the top rails below it; the tap sensor takes the 18B20's. The modem's 3.3 V
wire goes round the left of the PIR now, to keep clear of its wires.

<!-- bench A -->

<!-- steps A -->

??? info "Setting up the PIR and the obstacle sensor"
    As in Lesson 23: turn the PIR's time knob fully anticlockwise, leave its
    sensitivity in the middle, and give it a minute to settle after
    power-up. Turn the obstacle sensor's blue knob until its own LED just
    lights with your hand 10 cm away.

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B, in the den

Keep the modem, the screen and the clock module from Lesson 46, and take
out the RGB LED, its resistors and wires. The active buzzer goes beside the
screen in column 51, as in Lesson 24. The IR receiver's place beside the
screen is the clock module's, so it sits above the board a little further
along, over columns 38 to 40, powered from the top rails below it.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Open **File → Examples → Adk → Lesson47RemoteAlarm → Door** for Board A:

<!-- sketch A -->

What's new:

- Five `adk::Switch`es, each active high or low as its sensor says, as in
  Lesson 23. The tap sensor has no debouncing, the 0 at the end, as in
  Lesson 35: a knock is too short to wait for.
- `count ()` adds one to a tripwire's count when it goes off, and shares
  the count every pass; the bridge sends only a change. `long& times`
  means `times` is the very variable passed in, `moves` or `tilts`, not a
  copy, as `Player& player` did in Lesson 3, so `++times` adds to it.
- The tilt switch counts when it stops being upright:
  `upright.deactivated ()`.
- `bridge.value ("armed") == 1` is the den's setting, and lights the red
  LED. Until the den is heard, it is 0.

Then **File → Examples → Adk → Lesson47RemoteAlarm → Den** for Board B:

<!-- sketch B -->

What's new:

- `struct Tripwire` keeps each tripwire's name on the bridge, its name on
  the screen, and the latest count heard from it, starting at −1.
- `for (auto& wire : tripwires)` goes through the five, and
  `bridge.changed (wire.name)` is true when a new count has arrived.
  `heard ()` compares it with the one remembered: only a higher count is a
  trip. Then the bottom row gets the name and the time, and
  `sounding = sounding || armed;` starts the siren if the alarm is armed,
  and leaves it going if it already was.
- POWER on the remote swaps `armed` between `true` and `false` with `!`,
  and always stops the siren. `bridge.share ("armed", armed)` tells the
  door.
- `beat` ticks every 0.4 s, to rewrite the top row and, while the alarm
  sounds, beep.

## Upload it

1. Upload **Door** to Board A and **Den** to Board B, and keep out of the
   PIR's view for a minute while it settles.
2. The den's screen says `Disarmed Door ok` once the boards hear each
   other, and the door's green LED lights.
3. Try each tripwire: wave at the PIR, tip the tilt switch, push card
   into the beam-break sensor's slot, hold a hand in front of the obstacle
   sensor, and tap the tap sensor. Each time, within a moment, the bottom
   row names it with the time, say `Tilted  21:07:43`.
4. Press POWER. The top row says `Armed`, and the door's red LED lights.
   Set off a tripwire: `ALARM!` and the siren. Press POWER again to
   disarm it.
5. Press the den's reset button. When it starts again it hears the door's
   counts, but they are only where the counts stand: no tripwire shows, and
   no siren.

Now test your prediction: arm the alarm and unplug Board A. The siren stays
quiet: no count went up, because the door can't count anything any more.
Five seconds later the top row says `Armed   No news`, which is all the den
knows. A real alarm treats that silence as trouble too, and *Make it
yours* shows how.

## If it doesn't work

| What you see | Try this |
|---|---|
| `No news` all the time | The boards don't hear each other: check each modem as in Lesson 43. |
| A tripwire never shows | Check its wire: the PIR's OUT to A12, the tilt switch's A14 into a32, the beam-break's S to A15, the obstacle's OUT to pin 16, the tap's S to pin 17. Watch the module's own LED: if that doesn't light, check its + and GND on the rails. |
| A tripwire shows on its own, over and over | The PIR may be settling, or seeing warm air: give it a minute. The obstacle sensor may see too far: turn its knob back. |
| A tripwire shows when it stops, not when it starts | That module is the other way round from most: swap `adk::ActiveHigh` in or out of its line in **Door**. |
| POWER does nothing | Aim the remote at the receiver. Check its S goes to pin 2, + to T+39 and − to T-40. Lesson 22's sketch shows your remote's codes. |
| `ALARM!` but no sound | Check the buzzer's + leg in f51 under pin 12's wire in j51, and the black wire from a51 to the − rail. |
| The door's red LED never lights | Check pin 26's wire to j6 and the black wire from a7 to the − rail. It lights only while the den says armed. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The door shares five names and the den one, so the messages look like
    this, the door's first and then the den's:

    ```text
    @motion=7 tilt=2 beam=0 near=4 knock=12
    @armed=1
    ```

    After each change the bridge sends only the names that changed, such
    as `@knock=13`, and every two seconds everything again. A tap sensor
    rattles, so one knock may add two or three to its count, and the den
    may show `Knock` twice in one second; all it needs is to see the count
    rise.

    The den doesn't need the counts to be right, only to rise. That's why
    counting is such a sturdy way to send an event: a lost message, a
    doubled one or a late one still leaves the count higher than before.

## Make it yours

1. **Cut wires.** A burglar who unplugs the door board shouldn't win. In
   **Den**, sound the siren when the alarm is armed and
   `bridge.isConnected ()` turns false.
2. **A code.** Bring in Lesson 24's code: POWER arms the alarm, but only
   four digits on the remote disarm it.
3. **Which tripwires.** Leave some tripwires out of the alarm: the obstacle
   sensor could just chime once, with `siren.beep (50)`, while the others
   sound the siren.
4. **More tripwires.** The 37 in 1 kit has more on/off modules: the flame
   sensor, the touch sensor, the line-tracking sensor, the Hall sensor and
   the reed switch. Each is an `adk::Switch` too: swap one in for a sensor
   here and give it a name, up to seven letters. The Hall sensor and the
   reed switch need a magnet, and neither kit has one; a fridge magnet
   works.
5. **A tally.** Show each tripwire's count on the bottom row in turn, as
   Lesson 46 showed its readings.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Measure Board A, and
press POWER on the den's remote between readings.

!!! question "Predict"
    Pin 26 lights the door's red LED. What will it read while the den is
    disarmed, and once you press POWER in the den?

<!-- measure A -->

What the numbers tell you:

- **Pin 26** follows a button pressed in another room: 0 V disarmed and
  about 5 V armed. The press went from the remote to the den as light, and
  from the den to the door as a radio message, `@armed=1`.
- **The tilt switch's pin** reads 0 V upright and about 5 V tipped over,
  as in Lesson 23: the pull-up holds A14 high until the ball joins it to
  the − rail.
