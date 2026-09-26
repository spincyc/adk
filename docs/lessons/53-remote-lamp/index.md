---
lesson: 53
promise: Press the remote in one room, and a lamp clicks on in another, while a TV there obeys your other buttons.
time: 60 minutes
level: 3
parts:
  - "Board A: the Mega, breadboard and screen from Lesson 52"
  - "Board A: the IR receiver module and the kit's remote"
  - "Board B: the second Mega and breadboard from Lesson 52"
  - "Board B: the relay module (37 in 1), a 9 V battery and its snap lead, a red LED and a 1 kΩ resistor, as in Lesson 35"
  - "Board B: the IR LED module, KY-005 (37 in 1), and a 220 Ω resistor"
  - "Each board: its LoRa modem at the bridge's home, from Lesson 52"
  - "Board A: 19 jumper wires and 7 female-to-male; Board B: 5 jumper wires and 9 female-to-male"
  - A small screwdriver for the relay's terminals
ideas:
  - Light, then radio, then light again
  - A state and an event, shared in different ways
  - Sending a remote's code with an infrared LED
  - Which codes a TV will obey
---

## What you'll build

<!-- closeup B -->

A remote control that works through walls. Board A sits by the sofa with
its infrared eye. Press the remote's power button, and in another room
Board B's relay clicks and a red lamp lights; a moment later Board A's
screen says **Lamp is on**, because Board B said so. Press **1**, and
Board A's screen says **Sent 0x0C**: in the other room, Board B's
infrared LED sends the very same code again, so a TV there, or another
board with an IR receiver, obeys as if you were standing in front of it.

<!-- closeup A -->

## The idea

**Light, then radio, then light again.** Infrared is light, and light
stops at a wall. Radio goes through. So Board A catches the remote's
flashes and turns them back into what they mean: a button's command and
the remote's address, two numbers. The bridge carries the numbers through
the wall by LoRa. Board B turns them into light again, flashing its own
infrared LED with exactly the code the remote sent. Shops sell boxes that
do this, called IR extenders; here you have built one.

**A state and an event.** The lamp is a **state**: it is on or it is off,
and it stays that way. Board A shares it as `lamp`, 1 or 0, and if a
message is lost, the next one says the same thing and puts it right. A
button press is an **event**: it happens once, and then it's over. Sharing
only the button's code wouldn't work, because the bridge sends a number
only when it changes: press **1** twice and the code is 0x0C both times,
so the second press would never cross. So Board A also shares a count,
`presses`, which goes up by one with every press. Board B watches the
count, and sends a code each time it changes.

| Shared | By | Kind | The other board |
|---|---|---|---|
| `lamp` | A | a state, 0 or 1 | B keeps the relay on or off to match |
| `button`, `address` | A | the last press's code | B sends them with the IR LED... |
| `presses` | A | an event, counted | ...each time this changes |
| `relay` | B | a state, 0 or 1 | A shows it: the lamp really is on |

**Sending infrared.** The IR LED module is an LED whose light is
infrared, like the one at the end of the remote. Lesson 22 said a receiver
only answers light that flickers 38,000 times a second, so the LED has to
flicker that fast while it is lit. The Mega's Timer 3 does the flickering
by itself, on pins 2, 3 or 5; the sketch only switches the flicker on and
off in the pattern of the code, for about 67 ms. Pin 3 is the LED's pin
here, the one the dimmable LED of Lesson 7 used, and like that LED it goes
through a 220 Ω resistor.

**Which codes a TV will obey.** Every remote speaks to its own device.
The kit's remote sends its own codes, which mean nothing to a TV, so the
TV ignores them. But if your TV's remote speaks NEC, the language the
kit's remote speaks, Board A can read its codes too, and Board B repeats
them exactly, address and all. Point the TV's own remote at Board A and
the TV in the other room obeys. Many TVs speak NEC; many others don't.

!!! question "Predict"
    Press **1** on the remote, then press **1** again. Board A's `button`
    is 0x0C both times. Will Board B's LED send the code once, or twice?
    And if you hold **VOL+** down for two seconds, how many times will it
    send?

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables, and unclip the 9 V battery, before you
    change any wiring. Never let the battery's two terminals touch each
    other or anything metal.

!!! danger "Never mains electricity"
    The relay's label says it can switch 250 V. That is exactly why it
    only ever switches a 9 V battery and an LED here. Never connect it to
    anything that plugs into a wall socket, and never open up a lamp or a
    TV to wire it in: mains wiring can kill. [Safety](../../safety.md)
    says more.

Each board keeps its LoRa modem, its divider and their wires at the
bridge's home, just as in Lesson 52.

### Board A: the eye

Keep the screen. Take the knob out, with its three wires. The IR receiver
sits at its home beside the screen, above the board over columns 28 to 30,
as in Lesson 24, and takes its power from the top rails below it. The
wires from pins 14 and 15 go over the top of it.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: the lamp and the IR LED

Take out the stepper and its driver, and the power module: nothing on
this board needs more than the Mega can give. The relay and its lamp go
in just as in Lesson 35: the relay lies on its side above the board, its
three pins toward the Mega and its screw terminals away from it, and the
lamp, a 1 kΩ resistor and a red LED, stands in columns 13 and 14. The
9 V battery waits below the board: its black lead goes into a14, and its
red lead round to the relay's **COM** terminal.

The IR LED module goes below the board, beside the modem, with its LED
pointing away from the board. Its **S** pin takes pin 3 through the
220 Ω resistor, which stands across the middle gap in column 38, and its
**−** goes to the − rail. Its middle pin, marked **+** or nothing at all,
isn't needed: the LED lights from S to −. Leave it empty.

<!-- bench B -->

<!-- steps B -->

To fit a wire into one of the relay's screw terminals, loosen the screw a
few turns, push the wire's end into the square hole beneath it, and
tighten the screw until the wire can't be pulled out.

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Open **File → Examples → Adk → Lesson53RemoteLamp → Receiver** for
Board A:

<!-- sketch A -->

And **Lesson53RemoteLamp → Repeater** for Board B:

<!-- sketch B -->

What's new:

- `receiver.address ()` says which remote sent the code: 0 for the kit's
  remote, and a number of its own for each other remote that speaks NEC.
  `obey ()` keeps it with the command, so Board B can send both.
- `presses` counts the buttons passed on, and `++presses` adds one. The
  lamp's `lampOn` is a `bool`, which the bridge shares as 1 or 0.
- `bridge.changed ("presses")` on Board B is true in the one pass of
  `loop ()` in which a new count arrives. The `> 0` leaves out the first
  count Board A shares when it starts, 0, which is no press at all.
- `adk::IrTransmitter irLed {3};` is the IR LED on pin 3. It can only go
  on pin 2, 3 or 5, the pins Timer 3 can flicker. `irLed.send (command,
  address)` sends one press of a button, exactly as a remote would.
- `adk::hex (button, 2)` prints the code as two hexadecimal digits, 0x0C
  rather than 0xC, the way Lesson 22's table writes them. After the `#`
  comes the count, so you can see each press go.
- Board B shares `relay.isOn ()` back as `relay`, and Board A's screen
  shows that, not its own `lampOn`: the screen only says the lamp is on
  once the far board has switched it. The `? :` from Lesson 22 picks
  **on** or **off**.

## Upload it

1. Upload **Receiver** to Board A and **Repeater** to Board B, choosing
   each board's port in **Tools → Port**. Clip Board B's battery back on.
2. Board B's **L** LED lights within a couple of seconds, and Board A's
   screen says **Sent 0x00, #0**, nothing sent yet, and **Lamp is off**.
3. Aim the kit's remote at Board A and press **POWER**. Board B's relay
   clicks and its lamp lights, and Board A's screen changes to **Lamp is
   on**. Press it again to switch the lamp off.
4. Press **1**. Board A's screen says **Sent 0x0C, #1**. Board B's IR LED
   has just sent the code, but you can't see it: infrared is invisible.

To see the LED work, look at it through a phone's camera, as Lesson 22
suggested for the remote, and press a button: the camera shows the LED
flash, pale purple or white. Some phones' main cameras filter infrared
out; the selfie camera often doesn't.

To see it obeyed, point Board B's LED at something that listens, from a
metre or so away:

- **Another board.** If a friend has a third Mega running Lesson 22's
  sketch, point Board B's LED at its receiver and press **1** to **6** on
  your remote at Board A: the lamp there changes color, and its Serial
  Monitor prints each code.
- **A TV.** Point the TV's own remote at Board A and press a button. If
  Board A's screen shows a code, the remote speaks NEC, and the TV in the
  other room should obey its buttons. If the screen stays the same, the
  TV speaks another language, which ADK doesn't read.

Now carry Board B to another room, powered from a USB power bank or a
phone charger, and try it all again through the wall.

You predicted what two presses of **1** would do: Board B sends the code
twice. The count went from 1 to 2, and a change of count is a new press,
even though the code is the same. Holding **VOL+** sends it only once:
while a button is held, the remote sends short "still held" repeats, and
Board A leaves them out. To turn the TV up by five, press five times.

!!! question "Predict: an echo"
    With the two boards side by side, turn Board B's IR LED to face Board
    A's receiver, a hand's width away, and press **1** once. What will the
    count on Board A's screen do?

It climbs on its own, several times a second. Board A hears Board B's
copy of the code as a new press, and passes it on; Board B sends it again,
Board A hears it again, and round it goes: an **echo**, like a microphone
too close to its own speaker. It also proves Board B's LED works. Turn the
LED away and the echo stops.

## If it doesn't work

| What you see | Try this |
|---|---|
| **No word from B** | Is Board B powered and running **Repeater**? Its **L** LED lights when it hears Board A. Check both modems' wiring against the steps. |
| Pressing buttons changes nothing on Board A's screen | Check the receiver's S goes to pin 2, its + to the top + rail (T+29) and its − to the top − rail (T-30). Aim at its dark window, from closer. |
| **Lamp is on**, but the lamp is dark | Check the battery's red lead is tight in COM, the wire from NO goes to j13, the LED's long leg is in b13, and the battery's black lead is in a14. Is the battery flat? |
| The relay never clicks | Check S goes to pin 11, + to the inner 5V pin at the top of the long header, and − to the inner GND pin at its other end. |
| The lamp is on while the relay is off | The wire is in NC. Move it to NO. |
| The count goes up, but a phone camera never sees Board B's LED flash | Check pin 3's wire is in j38, the 220 Ω from g38 to e38, the module's S in a38 and its − in the − rail. Try another camera. |
| The camera sees it flash, but the TV or the other board ignores it | Aim straight at it, closer: this LED is weaker than a remote's. A TV only obeys codes from its own remote. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    When you press **1**, the remote flashes a 67 ms code, and Board A's
    receiver decodes it, as in Lesson 22, into command 0x0C (12) and
    address 0. Board A then shares all its values that changed in one
    message, such as:

    ```text
    @button=12 presses=1
    ```

    Board B's `bridge.changed ("presses")` is true for that one pass of
    `loop ()`, and `irLed.send (12, 0)` builds the same 32 bits the remote
    sent: the address, the address upside down, the command and the
    command upside down. It switches Timer 3's flicker on and off for each
    burst and gap, timing them from the start of the code. That takes
    about 67 ms, and `send ()` doesn't return until it's done, so Board B
    does nothing else meanwhile, as the ultrasonic sensor held the sketch
    for its echo in Lesson 19. The bridge simply catches up afterwards.

    The LED lights for a third of each flicker, about 17 mA while it is
    on, through the 220 Ω resistor. A TV remote pushes far more through
    its LED, which is why it reaches across a room and Board B's LED
    reaches a metre or two.

## Make it yours

1. **Hold to repeat.** Pass on held buttons too: count repeats as
   `repeats` on Board A, and on Board B call `irLed.repeat ()` each time
   that count changes. Now holding **VOL+** keeps the TV's volume going.
2. **A universal remote.** Find your TV's own codes with its remote and
   Board A's screen. Then make Board A pass on the TV's code when you
   press a button on the kit's remote: **VOL+** on the kit's remote could
   become the TV's volume up, address and all.
3. **A sleep timer.** Make Board A switch the lamp off again 30 minutes
   after it was switched on, with an `adk::Timer`, as in Lesson 35's last
   challenge.
4. **Two lamps.** Add a second relay and lamp to Board B, and let the
   remote's **EQ** button switch it: a second state, shared as a second
   name.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM**, the red one in **V**, never the **10A** socket.
Both readings are on Board B, in the lamp's own circuit, which only ever
carries the battery's 9 V; you switch the lamp from Board A.

!!! question "Predict"
    The relay's contact is the only thing joining the battery to the lamp.
    What will the first reading show with the lamp on, and with it off?

<!-- measure B -->

What the numbers tell you:

- **With the lamp on**, the meter reads the battery, about 9 V, straight
  through the relay's closed contact.
- **With the lamp off**, it reads 0: the contact is open, and nothing
  joins the battery to the lamp. The press that opened it came from
  another room, as light, then radio, then a coil's pull on a switch.
