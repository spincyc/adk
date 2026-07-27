# Changelog

## 0.3.0 — experimental

- Pinned local bootstrap to Arduino AVR core 1.8.8, added the missing Arch Git
  dependency, and made `serial-log` preserve monitor failures.
- Added newest-lesson site validation and least-privilege post-deployment
  checks for the landing page, HTML lesson, PDF, and canonical sketch.
- Added a deterministic native C++17 source export and clean value-consumer
  compile, static-link, and execution gate without changing Arduino packaging.
- Added shift-register, seven-segment, HD44780 character-display, and matrix
  keypad adapters with transactional ownership and deterministic host seams.
- Added validated climate samples, a bounded DHT11 transport, stable records,
  and the deterministic Environmental Station engine.
- Added explicit traffic timing and the fail-safe tabletop Traffic Junction.
- Replaced enum-style success comparisons with the compact `Status` value API:
  `ok()`, `error()`, and centrally classified `transient()`.
- Added narrative Mega examples and complementary monochrome HTML/PDF lessons
  through lesson 021.
- Added bounded servo intent, versioned configuration records, direct
  Timer5/D44 pulse ownership, and explicit external-power admission.
- Added the deterministic inert access trainer with keypad policy, visible
  lockout evidence, soft-latch intent, and bounded audit presentation.
- Added deterministic ultrasonic pulse timing with distinct valid, timeout,
  and out-of-range evidence.
- Added deterministic motor intent with bounded duty, reversal dead time, and
  stop-dominant fault behavior.
- Added deterministic rover supervision with range freshness, motion evidence,
  route replay, and an inert LED-only Mega stage.
- Added owned I2C/SPI devices, direct Mega AVR bus backends, calibrated moisture
  samples, explicit RTC state, and deterministic durable-record recovery.
- Added transactional inert-load indicators and deterministic watering policy
  with explicit recovery and exclusion evidence.
- Added the deterministic greenhouse trainer with coherent stages, visible
  health modes, durable record retry, and replay evidence.
- Added owned bounded infrared pulse capture, the Mega interrupt adapter,
  classic NEC-only decoding, and stable receive-only records.
- Added an exact receive-only telemetry packet, deterministic freshness and
  sequence tracking, bounded packet reception, and tested evidence scheduling.
- Added the deterministic telemetry console with configured source identity,
  explicit health, acknowledgement, and byte-stable bounded record retries.
- Added deterministic inert channel assessment and recorded observation
  fixtures with explicit open, short, stale, and contradictory evidence.
- Added deterministic inert cue scheduling with bounded operator confirmation,
  hold/cancel dominance, delayed-update coalescing, and byte-stable audit records.
- Added the physically inert show-cue simulator with complete continuity
  snapshots, deterministic fault and cancellation precedence, visible
  LED-only evidence, and replayable audit traces.
- Added executable USB route-controller and USB/IP research gates, plus the
  transparent USB and HDMI mesh architecture and safety boundaries.
- Added complete-kit inventory, component-family, safety, pacing, and project
  plans extending the curriculum without renumbering existing lessons.

Interfaces through lesson 030 are host verified and experimental. Physical
Mega 2560 acceptance, USB/HDMI endpoint hardware, interoperability, and
performance evidence remain open and are not implied by this version.

## 0.2.0 — experimental

- Added RAII `AnalogInput` for all Mega 2560 analog channels.
- Added deterministic calibration, moving-average, and dead-band processing.
- Added the adaptive Night Light engine with hysteresis and safe sensor faults.
- Added narrative examples, HTML, pencil diagrams, and PDFs for lessons
  007–009.
- Added Make-driven hardware evidence records and named upload workflows.
- Added installed-archive smoke tests, sanitizers, and versioned firmware
  budgets for all nine examples.
- Added feasibility and architecture studies for switched-network 8K HDMI and
  full USB 3 matrices, with the Mega constrained to their control plane.

Interfaces through lesson 009 are host verified and experimental. Physical
Mega 2560 acceptance remains open and is not implied by this version.

## 0.1.0 — experimental

- Isolated the original preview under `legacy/`.
- Added no-exception status, time, runtime, and resource claims.
- Added RAII digital output, digital input, mono LED, and Button interfaces.
- Added the deterministic Reaction Timer project.
- Added RAII PWM output, RGB LED, and nonblocking piezo sounder interfaces.
- Added the versioned, seeded, deterministic Simon engine.
- Added first-class lesson sources 001–006; hardware examples and acceptance
  remain gated separately.
- Added shared timer claims with a fixed 17-byte resource registry.
- Added host, firmware, documentation, packaging, and publication gates.

Interfaces through lesson 006 are host verified and experimental. Physical
Mega 2560 acceptance remains open and is not implied by this version.
