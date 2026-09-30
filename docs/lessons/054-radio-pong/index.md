---
lesson: 54
promise: Play Pong with a friend in another room, the ball flying off the top of your matrix and dropping onto theirs.
time: 2 hours
level: 3
parts:
  - "Each board: the Mega, breadboard and LoRa modem from Lesson 53"
  - "Each board: an LED matrix, a joystick, a passive buzzer and a 220 Ω resistor, wired as in Lesson 27"
  - "Not in one kit: the second matrix, joystick and passive buzzer, from a second kit"
  - "Each board: 4 jumper wires and 14 female-to-male"
  - A friend to play against
ideas:
  - Deciding which board owns the ball
  - Handing over with a count that changes
  - Mirroring, for a player who faces you
  - One sketch on two boards, one line apart
---

## What you'll build

<!-- closeup A -->

Pong, played between two rooms. Each player has a matrix, a joystick and
a buzzer. Your paddle is three dots along the bottom of your matrix;
push the stick left and right to move it. Click the stick to serve, and
the ball flies up, off the top of your matrix, and a moment later drops
in at the top of your friend's, in another room. They get their paddle
under it and send it back. Miss, and your buzzer groans while theirs
cheers, and both matrices scroll the score. Every hit makes the ball a
little faster. With nobody at the other end, Board A's top row is a wall, so you can
practice alone on Board A. Board B waits for an incoming ball.

## The idea

**One ball, two boards.** The game needs one ball, but there are two
boards, and each could move it. If both did, they would soon disagree
about where it was. So only one board ever moves the ball: the one whose
matrix it is on. Each board keeps the ball in one of three states:

| State | On this matrix | What changes it |
|---|---|---|
| **Serving** | The ball sits on the middle of your paddle and moves with it | Clicking the stick: the ball flies up, and the state is **Here**. |
| **Here** | The ball moves one dot every step, bouncing off the sides and your paddle | Leaving the top: the state is **There**. Getting past your paddle: a point to the other side, and you serve next. |
| **There** | Only your paddle: the ball is on the other matrix | The ball coming back over the bridge: **Here** again. |

At the start only Board A is **Serving**; Board B is **There**, waiting.
That fixed first server gives the game one ball even if both players click
at once. After a miss, only the player who missed may serve.

**Handing over with a count.** When the ball leaves the top of your
matrix, your board shares three things about it: the column it crossed
at, its **drift**, −1, 0 or 1 for which way it was going across, and its
**pace**, the milliseconds between steps. And it adds one to a count
called `ball`. The other board watches that count, and when it goes up,
the ball is its own. The count and all three flight details travel in
one `shareEvent ()` record, so a lost packet cannot attach old flight
details to a new crossing. The count is what matters: a ball can leave twice
from the same column, the same way, at the same pace, and then only the
count is new. Lesson 53 counted its presses the same way.

**Mirroring.** Picture a table between the two rooms, with the two
matrices at its ends and the players facing each other. Your left is
your friend's right. So a ball that leaves your matrix at column 1,
heading right, comes onto theirs at column 7 − 1 = 6, heading left as
they see it:

<p class="formula">their column = 7 − your column &nbsp;&nbsp; their drift = −your drift</p>

**The score is two counts.** Each board counts its own misses and shares
them. Your points are your friend's misses, and theirs are yours, so
both boards calculate the same score once both counts have arrived.

**One sketch, two boards.** Both boards run the same program. The only
line that differs is `firstServer`: true on Board A and false on Board B.
That choice sets both the starting ball owner and the radio addresses: A
is address 1 with partner 2; B is address 2 with partner 1. The two copies are in their
own folders, **Ping** and **Pong**, so you don't have to change anything
before uploading.

!!! question "Predict"
    The ball leaves the top of Board A's matrix at column 2, heading
    right. Where will it come in on Board B's matrix, and which way will
    it go, as Board B's player sees it? And which way is that across the
    room?

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables, and unclip Board B's battery, before
    you change any wiring.

Both boards end up built exactly alike: Lesson 27's matrix, joystick and
buzzer in the same places as in Lesson 27, and the LoRa modem at the
bridge's home, where it has been. What you take out differs: Board A
loses Lesson 53's screen and IR receiver, and Board B its relay, lamp,
battery and IR LED. The passive buzzer always goes through its 220 Ω
resistor.

### Board A

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B

Build Board B just like Board A, with its matrix, joystick and buzzer in
the same places. These steps start from Lesson 53's Board B:

<!-- bench B -->

<!-- steps B -->

## Code it

Open **File → Examples → Adk → lessons → 054-radio-pong → Ping** for Board A:

<!-- sketch A -->

Board B's sketch, **lessons → 054-radio-pong → Pong**, is the same but for
`firstServer`, which is `false` on Board B.

What's new:

- `enum class Ball { Serving, Here, There };` names the three states from
  the table above, as Lesson 24 named its alarm's.
- `wentUp ()` is Lesson 51's. `wentUp ("ball", theirCrossings)` is true
  when the other board's count of crossings goes up: a ball is coming.
  `theirCrossings` starts at −1. `firstBall` also accepts a first positive
  crossing count, because the startup zero may have been lost. For
  `theirMisses`, a first count remains only a baseline for the cheer.
  Reset both boards to start a new game; restarting one mid-rally can
  replay an old crossing. `catchBall ()` unpacks and mirrors the flight.
- `flight = pace * 32 + (drift + 1) * 8 + column` packs bounded whole
  numbers, as Lesson 53 packed an IR command: column is 0–7, drift + 1
  is 0–2, and pace is 100–250. `% 8` recovers the column, `/ 8 % 4 - 1`
  the drift, and `/ 32` the pace. The flight travels with its crossing
  count through `shareEvent ("ball", crossings, flight)`.
- The shared flight and miss count are kept apart from the ball's own
  `x` and `y`. The ball moves every step, but `column`, `drift`, `pace`
  and the counts only change when something happens, and the bridge only
  sends what has changed. So nothing goes over the air while the ball is
  in play on one side, but for the bridge's check every two seconds.
- `paddle + joystick.x () / 60` moves the paddle one dot every 80 ms
  while the stick is pushed: `joystick.x ()` runs from −100 to 100, and
  a whole-number division by 60 is 0 until it is pushed more than 60 of
  the way, then 1 or −1. `constrain ()`, from Lesson 26, keeps the
  three-dot paddle on the matrix, its left dot from 0 to 5.
- `random (-1, 2)` picks −1, 0 or 1 for the serve, as `random ()` picked
  in Lesson 3: never the top number itself.
- In `moveBall ()`, the ball bounces off a side by turning its drift
  round and stepping back inside. On row 6, the row above the paddle,
  moving down, over one of the paddle's three dots, it goes back up,
  its new drift `x - paddle - 1`: −1 over the left dot, 0 over the
  middle, 1 over the right. On row 7 it has got past.
  `ballStep.period (max (100UL, ...))` speeds up the beat as in Snake.
- `sendOver ()` shares the ball and hands it over. When
  `bridge.isConnected ()` is false, nobody would catch it, so it bounces
  back instead.
- `showScore ()` prints your points, a dash and the other player's into
  an `adk::Text`, and scrolls it, as Snake scrolled its score.
- `draw ()` makes a picture of eight rows, all dark to begin with, and
  lights dots in it: `0b11100000 >> paddle` is three dots shifted along
  to the paddle's place, as Lesson 30 shifted a dot along a row, and
  `rows[y] |= 0b10000000 >> x` adds the ball's dot to its row: `|=`
  lights the dots the right side has lit, and leaves the rest of the row
  as it was. `matrix.show (rows)` shows the picture, and only sends the
  rows that differ from what is showing. While the score scrolls, `draw ()`
  waits, unless a ball is in play here.
- The Mega's own **L** LED shows `bridge.isConnected ()`.

## Upload it

1. Upload **Ping** to Board A and **Pong** to Board B, choosing each
   board's port in **Tools → Port**.
2. Power each board from its own USB cable, a power bank or a phone
   charger, and put them in two rooms, or at least with their players
   back to back. Within a couple of seconds both **L** LEDs light.
3. Hold the joystick with its pins pointing to your left, as in Lesson
   27. Each paddle sits along the bottom row. Only Board A has the ball on it.
4. Board A's player clicks the stick. The ball flies up, off the top, and
   drops in at the top of the other matrix with a little blip. Hit it
   back, and keep the rally going.
5. Miss, and the buzzer groans; the other board cheers, and both scroll
   the score, each with its own points first. Whoever missed serves next.

You predicted where the ball comes in. Column 2 on Board A becomes
column 7 − 2 = 5 on Board B, and heading right on Board A means heading
left on Board B, as its player sees it. Across the room, though, it is
the same way: the two players face each other, so what is right for one
is left for the other. Stand the two matrices at the ends of a table and
you can see the ball's path carry straight on.

Now and then the ball may hang in the air for a second or two before it
comes down. The message that carried it was lost, and the bridge sent it
again with its next check. That's the radio, not you.

## If it doesn't work

| What you see | Try this |
|---|---|
| An **L** LED stays dark, and the ball bounces off the top | That board hears nothing from the other. Check that one board runs **Ping** and the other **Pong**: two Pings are both address 1, and neither hears the other. Check both modems' wiring against the steps. |
| The ball goes over, but never comes down on the other matrix | Wait two seconds for the bridge to send it again. If the other **L** LED is dark, that board has lost the link. |
| The paddle moves the wrong way | Hold the joystick with its pins to your left. |
| The picture is upside down or back to front | Turn the matrix, as Lesson 25 says, until the paddle is along the bottom. |
| Two balls at once | Check `firstServer`: true only on Board A, false on Board B. Reset both boards after correcting it. |
| The score looks wrong after a board was reset | Each board counts its own misses from when it started. Press both reset buttons together for a new game. |
| The ball never comes back, and your board's **L** LED is dark | The other board may have been switched off with the ball. Reset both boards for a new game. To practise alone, reset Board A: it is the first server. |
| No sound | Check the buzzer's + leg is in f33, beside pin 10's wire in j33, and its 220 Ω from a33 to the − rail. |
| The matrix shows junk | Check its wires, especially CLK on 48 and CS on 49. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    While the ball is on one side, the bridge has nothing to send, so the
    air is quiet but for each board's check every two seconds. When the
    ball crosses, Board A's modem sends one message:

    ```text
    @ball=12:6098
    ```

    Here `6098 = 190 × 32 + (1 + 1) × 8 + 2`: pace, drift and column
    together. A miss sends `@misses=3`. That is the whole game on the
    air: a few messages a rally, each in about a twentieth of a second.
    Sending the ball's place at every step instead would keep both modems
    busy, and two modems sending at once lose both messages.

    Every two seconds each board repeats everything it shares. If a
    handover was lost and a later repeat arrives, the complete crossing
    is still new to the other board, so the ball comes down late. Repeats
    improve the chance of delivery; the bridge does not acknowledge or
    guarantee delivery. Persistent loss can stop a rally.

    Both players cannot serve at startup: Board B starts in `There` and
    ignores its serve button until it has received and missed a ball.
    Choosing one initial owner avoids two handovers crossing in flight
    and leaving both boards with a ball. Reset both boards for a new game.

## Make it yours

1. **First to five.** When either score reaches 5, play a tune and
   scroll *YOU WIN* on one matrix and *YOU LOSE* on the other. Both
   boards know both counts, so they agree who won.
2. **A smaller paddle.** After ten hits in a rally, make the paddle two
   dots wide instead of three, and three again after a miss.
3. **Hear them hit.** Count your paddle's hits and share the count; when
   the other board's count goes up, play a soft tick, so you hear the
   ball being hit back before it arrives.
4. **Aim the bounce.** If the paddle is moving when it hits the ball,
   set the drift to −1 or 1 in that direction. Keep it between −1 and 1:
   those are the limits of the packed flight record.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take the readings on
Board A while no ball is in play, before either of you serves, so the
serial line to the modem rests. Both boards read the same.

!!! question "Predict"
    Pin 14, TX3, rests at 5 V between messages. What will the modem's RXD
    read, on the other side of the 1 kΩ?

<!-- measure A -->

What the numbers tell you:

- **Pin 14** rests at about 5 V: a serial line sits high while it has
  nothing to say. When a message goes out, it flickers far too fast for
  the meter, and the reading only dips a little.
- **The modem's RXD** reads about 3.3 V, two thirds of 5 V: the 1 kΩ and
  2 kΩ share the 5 V between them, so the modem's pin never sees more
  than it can take. It is the same divider as on every board since
  Lesson 40.
