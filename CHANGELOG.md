# Changelog

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
