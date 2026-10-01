# Power and heat

Every part that carries a current turns some energy into light, sound,
motion or heat. **Power** says how fast. It is why a resistor has a
rating, why a short circuit gets hot, and why a motor never runs from a
pin. No investigation measures power directly; each lesson works it out
from a voltage and a current.

## The power law {#power}

<!-- law power -->

With [Ohm's law](ohms-law.md), a resistor's power can be found from any
two of its voltage, current and resistance: I² × R needs only the current
and the resistance, and V² ÷ R only the voltage and the resistance.

**Worked: Lesson 1's LED.** The resistor has 3 V across it and 14 mA
through it: 3 V × 14 mA = 42 mW, about 0.04 W. The LED has 2 V at the
same current, 28 mW, and turns most of it into light. Volts times
milliamps gives milliwatts, as [the shortcuts](units.md#shortcuts) say.

Energy is power times time: a watt for one second is a joule. A power
bank's capacity, in milliamp-hours, is counted at its battery's own
3.7 V or so, so at 5 V a 10 000 mAh bank gives roughly 6000 mAh: 100 mA
for about 60 hours.

## Ratings and current budgets {#budgets}

<!-- law budgets -->

Each part can turn only so much power into heat before it is damaged.
Small through-hole resistors, like the kit's, are usually rated ¼ W, and
the course keeps every one well under that: the busiest, E12's 220 Ω
buzzer resistor at about 21 mA, takes 0.021² × 220 ≈ 0.1 W, under half
its rating.

Other parts give their limits as a current or a voltage instead:

| Part | Limit | Where it matters |
|---|---|---|
| A Mega I/O pin | 20 mA to work by; 40 mA is the absolute maximum, not a target | Every LED and buzzer driven from a pin |
| The Mega's chip, all pins together | About 200 mA, the absolute maximum through its supply pins | Many LEDs lit at once |
| The Mega's 3.3V pin | About 50 mA | The RFID reader and 3.3 V radio modules |
| USB into the Mega | About 500 mA, guarded by a fuse on the board | Everything on the 5 V rail together |
| The breadboard power module | About 700 mA | Servos, motors and steppers |
| An electrolytic capacitor | Its printed voltage, such as 16 V, and the right way round | E07, E08, E16 and the others that use one |

[Safety](../safety.md) gives the pin limit the course designs to. To
check a supply, add up what every part on it wants, using [Kirchhoff's current law](kirchhoff.md#current-law),
and compare it with what the supply can give. Motors, servos and steppers
want far more than a pin can give:

| Part | Wants | So the course |
|---|---|---|
| The fan's small DC motor | About 200 mA running, several times that starting | Drives it through an L293D from the power module (Lesson 20) |
| A servo in motion | Several hundred milliamps | Feeds it from the power module (Lesson 21) |
| The stepper's coils | Up to about 200 mA | Switches them with a driver board fed by the power module (Lesson 31) |
| A LoRa modem sending | Up to about 50 mA | Gives the modems the power module's 3.3 V (Lesson 40) |

A pin only ever *tells* such a part what to do; the current comes from a
supply that can give it, through a transistor or driver chip
([Diodes and transistors](diodes-transistors-op-amps.md#transistor-switch)).

## A short circuit {#short-circuit}

A wire has almost no resistance, so a wire straight from + to − lets the
supply push as much current as it can. That current, at the full supply
voltage, is a lot of power in a very small place:

- **A 10 Ω resistor across 5 V**, if a jumper in E05 or E18 went into the
  wrong rail: 5 V ÷ 10 Ω = 0.5 A, and 5 V × 0.5 A = 2.5 W, ten times a
  ¼ W rating. It would be too hot to touch within seconds.
- **A bare wire across 5 V:** only the USB fuse limits it, and it heats
  the wire and the supply instead.

This is why every page says to unplug before changing wires, and to
unplug at once if a part gets warm or smells: see [Safety](../safety.md).
It is also why an LED never goes in without its resistor: on its own, it
behaves almost like a short once its voltage is reached.

## Check yourself

1. How much power does E03's 2 kΩ resistor take, with about 3 V across
   it?
2. Could one Mega pin light three red LEDs side by side, each with its
   own 220 Ω resistor?
3. Why does Lesson 21 feed its servo from the power module, not from the
   Mega's 5V pin?

??? note "Answers"
    1. 3 V ÷ 2 kΩ = 1.5 mA, and 3 V × 1.5 mA = 4.5 mW, a fiftieth of a
       ¼ W rating.
    2. No: each takes about 14 mA, so together about 42 mA, twice a pin's
       20 mA. Give each its own pin, use larger resistors, or switch them
       with a transistor.
    3. A moving servo can want several hundred milliamps. Through the
       Mega, that would share USB's 500 mA with the board itself and the
       rest of the build, and each jolt would dip the 5 V everything else
       runs on.

## Lessons that rely on it

<!-- relied on -->
