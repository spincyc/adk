# Lesson @LESSON@ Mega 2560 acceptance

Status: draft; this file is not evidence until a named person records measured
observations and reviews the completed card.

## Bench identity

- Date and operator:
- Board and revision:
- Supply and current limit:
- Meter, oscilloscope, or logic analyzer:
- Sketch commit:
- Upload command:

## Unpowered wiring audit

- Energy class:
- Component values and ratings:
- Pin map:
- Common-ground check:
- Power-removal method:
- Stop conditions:

## Resource acquisition evidence

Record the circuit-native indication that initialization acquired its resources.
Do not infer this result from the safe output state.

| Signal or test point | Prediction | Observation | Interpretation |
|---|---|---|---|

## Safe-state evidence

Measure startup, shutdown, reset, and power removal at the named electrical test
point. A dark LED alone does not prove high impedance.

| Signal or test point | Prediction | Observation | Interpretation |
|---|---|---|---|

## Analog observations

For lessons 007--009, compare voltage at the sensor test point with the raw ADC
sample and the circuit-native output. Serial may supply the raw sample, but the
meter and visible output are independent evidence.

| Test point | Expected V | Measured V | Raw sample | PWM or visible output |
|---|---:|---:|---:|---|

## Fault and boundary observations

- Ground opened:
- Input at lower rail:
- Input at upper rail:
- Reset while active:
- Shutdown:

## Review

- Deviations:
- Remaining uncertainty:
- Reviewer and date:
- Result: draft / rejected / hardware verified
