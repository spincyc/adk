# Waves, PWM and sampling

Voltages that change in time: how often they repeat, how a pin that can
only switch makes something in between or tells a servo where to turn,
how the Mega turns a voltage into a number, and how a capacitor treats
slow and fast changes differently. Bits on a wire are on
[Logic levels and serial](digital.md#serial).

## Frequency and period {#frequency}

<!-- law frequency -->

A wave that repeats has a **period** T, the time for one cycle, and a
**frequency** f, how many cycles fit in a second. Each is one over the
other: f = 1 ÷ T, and T = 1 ÷ f.

| Frequency | Period | In ADK |
|---|---|---|
| 1 Hz | 1 s | A blink each second |
| 100 Hz | 10 ms | E14's slow wave |
| 440 Hz | about 2.3 ms | The note A that a tuning fork plays |
| 490 Hz | about 2 ms | Most of the Mega's PWM pins |
| 1 kHz | 1 ms | E14's fast wave |

For sound, frequency is **pitch**: Lesson 5's buzzer plays a higher note
by switching faster, and Lesson 9 doubles a note's frequency to go up an
octave. E14 counts cycles on a scope and checks the formula.

## PWM: an average from on and off {#pwm}

<!-- law pwm -->

A Mega pin is either 0 V or 5 V. To dim an LED it switches between the
two hundreds of times a second, and changes the fraction of each cycle
it spends on, the **duty cycle**. Whatever smooths the switching, an eye,
a meter or a capacitor, sees the average.

ADK's brightness and `analogWrite` levels go from 0 to 255, so a level of
64 is on 64 ÷ 255, about a quarter of the time, for an average near
1.25 V. Lesson 4 mixes colors this way and Lesson 7 dims an LED with a
knob. E23 smooths a pin's PWM with 10 kΩ and 100 µF and measures the
average as a steady voltage (see [τ = R × C](capacitors-and-coils.md#rc-time)).

A meter on a PWM pin shows the average; a scope shows the pin switching
between 0 and 5 V the whole time. That is why so many lessons' *Measure
it* read a fraction of 5 V on a pin that is busy switching, including a
buzzer's pin, at about half.

## Servo pulses {#servo-pulses}

<!-- law servo-pulses -->

A servo is told where to turn by pulses too, but it reads them
differently: not their average, but **how long each one lasts**. Every
20 ms the Mega sends one pulse, and the servo turns to the angle that
pulse's width stands for, in proportion between the two ends. Halfway,
90°, is:

<p class="formula">544 µs + <span class="fraction"><span>90</span><span>180</span></span> × (2400 µs − 544 µs) = 1472 µs</p>

Lesson 17 works this out. The pulse carries only the
message; the servo's current comes from the power module, as
[Power and heat](power.md#budgets) explains.

## Sampling: from a voltage to a number {#sampling}

<!-- law sampling -->

The Mega's **analog-to-digital converter** measures a voltage from 0 to
5 V on pins A0 to A15 and gives a number from 0 to 1023. Each step of one is about 5 V ÷ 1024 ≈ 4.9 mV, the smallest change it
can see: 2.5 V reads about 512, 1 V about 205. Working back, a reading
of 600 is about 600 × 4.9 mV ≈ 2.9 V.

Each reading is a **sample**, one snapshot. To follow a changing voltage
the samples must come often enough: at least twice in every cycle of the
fastest change you want to see, and better ten times. E22 compares the
smooth turn of a knob with the whole steps of the numbers; Lessons 8 and
48 read light and sound this way.

## Filters: slow and fast changes {#filter}

<!-- law filter -->

A capacitor lets a changing voltage through more easily the faster it
changes. Its opposition to a wave of frequency f is its **reactance**,
in ohms:

<p class="formula">X<sub>C</sub> = <span class="fraction"><span>1</span><span>2π × f × C</span></span></p>

Large for slow waves, small for fast ones. With waves, Ohm's law becomes
V = I × Z, where the **impedance** Z combines resistance and reactance.

Put a resistor in series and the capacitor across the output, and you
have a **low-pass filter**: slow changes pass, fast ones are smoothed
away. The change-over, the **corner frequency**, is where the reactance
equals the resistance: f<sub>c</sub> = 1 ÷ (2π × R × C), the formula
above.

E15's 1 kΩ and 1 µF have a corner at 1 ÷ (2π × 1000 × 0.000001) ≈
159 Hz. At 100 Hz, below it, the output is a little smaller than the
input; at 1 kHz, six times above it, it is about a sixth as large. The
output's peaks also come **later** than the input's, a shift called
**phase**, because the capacitor's charge takes time to follow.

Swap the two and the capacitor in series passes fast changes and blocks
the steady part: a **high-pass** filter. That is what E13's capacitor
does to the generator's 2 V offset, and what a scope's AC coupling does
to show a small ripple on a 5 V rail.

## Check yourself

1. A buzzer note repeats every 2 ms. What is its frequency?
2. What average voltage does a PWM level of 191 give?
3. A thermistor divider reads 307 on A0. About what voltage is that?
4. What is the corner frequency of 10 kΩ and 100 µF, as in E23? Why
   does it smooth 490 Hz PWM so well?
5. A servo pulse lasts 2400 µs. Where does the servo turn?

??? note "Answers"
    1. 1 ÷ 0.002 s = 500 Hz.
    2. 191 ÷ 255 ≈ 0.75, so about 3.75 V.
    3. 307 × 4.9 mV ≈ 1.5 V.
    4. 1 ÷ (2π × 10 000 × 0.0001) ≈ 0.16 Hz. 490 Hz is three thousand
       times higher, so almost nothing of the switching gets through,
       only the average.
    5. To 180°, the end of its travel.

## Lessons that rely on them

<!-- relied on -->
