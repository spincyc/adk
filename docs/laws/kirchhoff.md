# Kirchhoff's laws, series and parallel

Two rules about whole circuits, one for voltage and one for current, and
the shortcuts they give for parts in a row and parts side by side. With
Ohm's law they are enough to work out every voltage and current in any
circuit of resistors on a steady supply. They are named after Gustav
Kirchhoff, who wrote them down in 1845. If you are working through the
electricity course, do E02 to E05 first: this page gives away what they
find.

## The voltage law {#voltage-law}

<!-- law voltage-law -->

Go once around any closed loop: the voltage the supply gives equals the
voltages the parts take. In Lesson 1's LED loop, 5 V from the pin = about
3 V across the resistor + about 2 V across the LED. It holds because a
voltage is energy per coulomb: whatever a coulomb gains in the supply it
gives up on the way round, and it ends where it started.

E02 measures it: the supply, the resistor and the LED, and the two part
readings add up to the supply's. E04 does it with two resistors.

- **Find the voltage left for a resistor:** subtract every other part's
  share from the supply, as in [choosing an LED's resistor](ohms-law.md#led-resistor).
- **Check a set of readings:** around a loop they must add up. If they
  don't, a probe was on the wrong strip, which is the first check in
  [the diagnosis guide](../electricity/diagnose.md).
- **Parallel parts have the same voltage:** two branches between the
  same two strips make two loops with the same supply, so each branch
  gets the whole voltage between those strips.

## The current law {#current-law}

<!-- law current-law -->

Charge doesn't pile up in a wire or vanish from it, so every milliamp
that arrives at a junction leaves it again. In a single loop with no
branches the same current flows through every part: what goes through
the resistor goes through the LED. Where a path splits, the branch
currents add up to the current that fed them.

E05 measures it, with the same LED-and-resistor branch twice:

<!-- drawing 060-branches-in-parallel closeup -->

The current through the shared 10 Ω feed doubles, from about 3 mA to
6 mA, as each branch takes its own 3 mA.

- **Total a pin's or a supply's load:** a pin lighting two LEDs gives the
  sum of their currents, and that sum must stay under the pin's 20 mA.
  The same sum, over every part on a rail, is what
  [the supply must give](power.md#budgets).
- **Find a current you can't measure:** if you know all the currents into
  a junction but one, the last is whatever balances it. E10 does this at
  the transistor's base, where the base resistor's current splits between
  the base and the pull-down.

## Resistors in series {#series}

<!-- law series -->

Parts are **in series** when they sit one after another in a single path.
The current law says the same current passes through each; the voltage
law says their voltages add; so their resistances add too.

Two 1 kΩ resistors in a row behave like one 2 kΩ, which is how E04 finds
its current: 5 V ÷ 2 kΩ = 2.5 mA. Lesson 5's buzzer coil is about 16 Ω,
and its 220 Ω resistor in series makes 236 Ω, which keeps the current
from a pin to about 21 mA while the pin is HIGH, about 10 mA averaged
over each note, instead of about 310 mA. E08 doubles its charging
time by putting a second 10 kΩ in series.

Each part's share of the voltage is that current times its own
resistance, so the larger resistor takes the larger share: with 1 kΩ and
2 kΩ, about 1.7 V and 3.3 V. That is a [voltage divider](dividers.md).

## Resistors in parallel {#parallel}

<!-- law parallel -->

Parts are **in parallel** when each joins the same two nodes, side by
side, so each has the same voltage across it. Each takes its own current,
and the currents add, so together they let more current through than
either alone: their combined resistance is **smaller than the smallest**.
For any number of resistors:

<p class="formula"><span class="fraction"><span>1</span><span>R</span></span> =
<span class="fraction"><span>1</span><span>R₁</span></span> +
<span class="fraction"><span>1</span><span>R₂</span></span> + …</p>

and for two, the product over the sum, as above, is quicker:

- **Two equal resistors** make half of one: 1 kΩ and 1 kΩ make 500 Ω.
  Twice the paths, at the same voltage, carry twice the current. The
  [loaded-divider challenge](../electricity/challenges.md#loaded-divider)
  uses this.
- **A small one beside a large one** is close to the small one: 1 kΩ and
  4 kΩ make 1 × 4 ÷ 5 = 0.8 kΩ, which E17 uses.

## Solving a circuit with all three {#solving}

1. Trace the circuit and name its nodes: a node is a strip of holes
   together with every strip a wire joins to it, as [Read a schematic](../electricity/schematics.md#follow-the-nodes)
   explains.
2. Write down what you know: the supply voltage, each resistance, and any
   part that keeps a fixed voltage, such as an LED's 2 V.
3. Use the voltage law around each loop to find the voltage across each
   resistor.
4. Use Ohm's law to turn each resistor's voltage into its current.
5. Use the current law at each junction to add currents where paths meet.
6. Check: every loop adds up, every junction balances, and no part is
   past its limit.

**Worked: E05's two branches.** The feed is 10 Ω from 5 V, then two
branches, each a 1 kΩ resistor and a red LED, to GND.

- The voltage law around one branch: 5 V = the feed's voltage + the
  1 kΩ's voltage + the LED's 2 V. The feed takes only a few tens of
  millivolts, so the 1 kΩ gets about 3 V.
- Ohm's law: 3 V ÷ 1 kΩ = 3 mA in each branch.
- The current law where the branches meet the feed: 3 mA + 3 mA = 6 mA.
- Ohm's law for the feed: 6 mA × 10 Ω = 60 mV, the reading E05
  predicts.

## Check yourself

1. A meter reads 4.9 V from + to −, 2.0 V across an LED and 2.6 V across
   its resistor, all in one loop. What does the gap between the sum and
   the supply tell you?
2. Three red LEDs, each on its own 1 kΩ resistor, hang from the 5 V
   rail.
   How much current does the rail give them together?
3. In E04 two 1 kΩ resistors share 5 V. A meter reads 2.6 V across one.
   Without measuring, what should the other read?
4. What single resistor behaves like 220 Ω and 1 kΩ in series? And in
   parallel?

??? note "Answers"
    1. 2.0 V + 2.6 V = 4.6 V, which is 0.3 V short of 4.9 V. Around a loop
       the readings must add up, so a probe was probably on the wrong
       strip, or something else in the loop, such as a loose wire, has the
       missing 0.3 V across it.
    2. Each takes about 3 mA, so the rail gives 3 + 3 + 3 = 9 mA.
    3. About 5 V − 2.6 V = 2.4 V, if the supply is 5 V. Measure the supply
       too: the two shares add up to whatever it really is.
    4. In series, 1220 Ω. In parallel, 220 × 1000 ÷ 1220 ≈ 180 Ω, less
       than the smaller of the two.

## Lessons that rely on them

<!-- relied on -->
