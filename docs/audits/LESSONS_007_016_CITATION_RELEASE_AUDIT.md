# Lessons 007--016 citation and release audit

Date: 2026-07-27

Scope: lesson PDFs and HTML through 016, component/project status pages, kit
expansion plans, and the two network-matrix research constraints. This is an
independent release audit, not physical acceptance.

## Result

**Conditional fail.** Safety boundaries and evidence-status language are
generally strong, but the current tree should not be published as a finished
release until the status pages and citations below are corrected. No lesson
claims Mega 2560 bench acceptance. That accurate limitation must remain.

The repository commit audited was `aaf8fce`. It was ahead of `origin/main`
(`f6a587b`) during the audit. Seven links to new source/example/site artifacts
returned 404 because the referenced paths were not yet on the public `main` or
Pages site. Those are prepublication failures, not evidence that publication
will fix them; rerun the link checker after push and Pages deployment.

## Lesson findings

| Lesson | Safety and evidence status | Citation/source status | Disposition |
|---:|---|---|---|
| 007 | Correctly separates meter, ADC, PWM, acquisition, and safe-state evidence; no physical claim | Mega and Microchip primary pages are named | Pass after final link check |
| 008 | Correctly keeps calibration/filtering distinct and preserves raw evidence | Board/MCU sources are primary, but transformation policy is primarily project-authored | Pass |
| 009 | Correctly treats the LDR as relative and requires an identified part; bench remains open | No particular LDR datasheet can be cited until the specimen is selected | Pass as experimental, not hardware-supported |
| 010 | Current-limited E1 circuit; explicitly admits arbitrary 74HC595 power-up output with OE tied low | No exact 74HC595 or display primary datasheet is cited; public sketch link returned 404 | Block hardware promotion; experimental publication only |
| 011 | Low-voltage model is explicitly not a road controller; current arithmetic is conservative | Generic LEDs require actual part evidence only for bench acceptance | Pass as experimental |
| 012 | Inert traffic model and open bench status are clear; startup-output limitation is repeated | No exact 74HC595/display primary sources; four GitHub links returned 404 before publish | Block hardware promotion; experimental publication only |
| 013 | Transport, decoded validity, freshness, and accuracy are well separated | HTML says “a supported DHT11-compatible” module while exact identity and bench acceptance remain open; PDF/HTML provide no accepted DHT11 datasheet supporting timing and range constants | Release wording/source blocker |
| 014 | Backlight remains disconnected when its rating is unknown; retained pixels are not safe-state proof | No exact LCD module or HD44780-controller primary source; “compatible” is too broad to establish timing, pinout, voltage, or backlight limits | Experimental only; block hardware promotion |
| 015 | Correctly disclaims calibrated monitoring and life-safety use | Inherits unresolved DHT11 and LCD identities and lacks direct primary module references | Experimental only; block hardware promotion |
| 016 | Passive matrix, unpowered tail discovery, ghosting limitation, and non-security boundary are sound | An exact keypad datasheet is optional for continuity-derived mapping but exact specimen/revision remains required on the bench card | Pass as experimental |

## Release-claim defects

1. `docs/COMPONENTS.md` still says lesson 009 is the implemented slice even
   though the tree and curriculum report host-verified work through 016.
2. `docs/PROJECTS.md` says lesson 013 is active while `docs/CURRICULUM.md`
   correctly says lesson 017 is the active implementation boundary.
3. `site/pages/lessons/013.md` uses `supported` for an unidentified
   DHT11-compatible module. Use `candidate` or `identified` until its primary
   documentation and bench card land.
4. Lesson 010's downloadable sketch and lesson 011--012 source/example links
   were public 404s at audit time. The local files existing is insufficient;
   verify deployed URLs after Pages reaches the audited commit.
5. “Host verified” must continue to mean deterministic software evidence only.
   Do not turn it into “hardware verified,” “electrically safe,” or “supported
   module” in release notes.

## Broken or weak web references

The automated pass checked 66 distinct links found in lessons 007--016 and
adjacent design/project documents:

- 53 returned HTTP 200;
- seven project-owned GitHub/Pages paths returned 404 before publication;
- Arduino's former `Datalogger` built-in-example URL returned 404;
- the HDMI 2.1 URL used in `docs/PROJECTS.md` returned 404;
- the NIST finite-state-machine publication URL returned 404;
- Analog Devices DS18B20 and ST L298 datasheet requests returned no HTTP status
  in the automated client;
- Microchip product pages returned 403 to the automated client.

A 403 or client timeout does not prove a human-facing source is gone, but it
does make automated release checking unreliable. Prefer a stable manufacturer
PDF or an official landing page known to permit retrieval, record the document
revision, and keep the claim no broader than the retrieved source.

The isolated shared-constraints documents were corrected during this audit:
the obsolete HDMI 2.1 link and an unavailable Gravity 37-part store listing
were removed. Their remaining 19 references returned HTTP 200.

## Source sufficiency

Primary-source sufficiency is not merely a live-link test:

- Mega electrical limits require the Arduino board documents and ATmega2560
  datasheet, not a tutorial.
- DHT11 transaction timings, acquisition interval, value encoding, operating
  range, pull-up, and supply require the exact sensor/module revision.
- Shift-register output current, aggregate current, power-up state, VIH/VIL,
  timing, and decoupling require the exact logic-family manufacturer's
  datasheet.
- LCD controller timing does not establish a module's backlight resistor,
  polarity, operating voltage, or pin order.
- A keypad can be mapped by unpowered continuity, but that observation must be
  recorded against the exact specimen and cannot support anti-ghosting or
  security claims.
- Kit store inventories prove only what a named SKU listing says it contains.
  They do not prove the identity, ratings, condition, or ownership of a
  specimen.

## Minimum release actions

1. Reconcile `COMPONENTS.md`, `PROJECTS.md`, and `CURRICULUM.md` to one active
   boundary and one evidence vocabulary.
2. Replace `supported DHT11-compatible` with non-promotional experimental
   wording.
3. Either select exact 74HC595, display, DHT11, LCD, and keypad specimens with
   primary documents, or keep their hardware status explicitly open and avoid
   exact-device performance claims not traceable to a source.
4. Replace the three dead external references or remove the claims that need
   them.
5. Publish, wait for Pages, then require every internal lesson, sketch, source,
   PDF, and test link to return success at the deployed commit.
6. Preserve all blank bench records as blank. Publication, compilation, host
   tests, size evidence, and a successful link audit do not fill them.
