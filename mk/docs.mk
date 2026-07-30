LESSON_PDFS := $(addprefix doc/lessons/,$(addsuffix .pdf,$(LESSONS)))

.PHONY: lessons lessons-check
lessons: $(LESSON_PDFS)

doc/lessons/001.pdf: docs/lessons/001/main.tex \
		docs/lessons/assets/001-digital-output-pencil.png
doc/lessons/002.pdf: docs/lessons/002/main.tex \
		docs/lessons/assets/002-digital-input-pencil.png
doc/lessons/003.pdf: docs/lessons/003/main.tex \
		docs/lessons/assets/003-reaction-timer-pencil.png
doc/lessons/004.pdf: docs/lessons/004/main.tex \
		docs/lessons/assets/004-pwm-rgb-pencil.png
doc/lessons/005.pdf: docs/lessons/005/main.tex \
		docs/lessons/assets/005-piezo-pencil.png
doc/lessons/006.pdf: docs/lessons/006/main.tex \
		docs/lessons/assets/006-simon-pencil.png \
		docs/lessons/assets/006-simon-composition-pencil.png \
		docs/lessons/assets/006-simon-state-pencil.png
doc/lessons/007.pdf: docs/lessons/007/main.tex \
		docs/lessons/assets/007-analog-input-pencil.png
doc/lessons/008.pdf: docs/lessons/008/main.tex \
		docs/lessons/assets/008-sampled-signal-pencil.png \
		docs/lessons/assets/008-evidence-chain-pencil.png
doc/lessons/009.pdf: docs/lessons/009/main.tex \
		docs/lessons/assets/009-mega-header-locator.png \
		docs/lessons/assets/009-mega-header-locator.svg \
		docs/lessons/assets/009-night-light-overview.png \
		docs/lessons/assets/009-night-light-overview.svg \
		docs/lessons/assets/009-night-light-breadboard.png \
		docs/lessons/assets/009-night-light-breadboard.svg
doc/lessons/010.pdf: docs/lessons/010/main.tex \
		docs/lessons/assets/010-shift-register-pencil.png
doc/lessons/011.pdf: docs/lessons/011/main.tex \
		docs/lessons/assets/011-mega-header-locator.tex \
		docs/lessons/assets/011-timed-traffic-layout.tex \
		docs/lessons/assets/011-build-stages.tex
doc/lessons/012.pdf: docs/lessons/012/main.tex \
		docs/lessons/assets/012-traffic-junction-pencil.png \
		docs/lessons/assets/012-traffic-junction-pencil.svg
doc/lessons/013.pdf: docs/lessons/013/main.tex
doc/lessons/014.pdf: docs/lessons/014/main.tex \
		docs/lessons/assets/014-mega-lcd-pin-map.png \
		docs/lessons/assets/014-lcd-breadboard.png \
		docs/lessons/assets/014-lcd-visible-states.png
doc/lessons/015.pdf: docs/lessons/015/main.tex
doc/lessons/016.pdf: docs/lessons/016/main.tex \
		docs/lessons/assets/016-matrix-keypad-pencil.png
doc/lessons/017.pdf: docs/lessons/017/main.tex \
		docs/lessons/assets/017-bounded-servo-pencil.png
doc/lessons/018.pdf: docs/lessons/018/main.tex \
		docs/lessons/assets/016-matrix-keypad-pencil.png \
		docs/lessons/assets/018-indicator-breadboard.png \
		docs/lessons/assets/018-lcd-wiring.png \
		docs/lessons/assets/018-visible-states.png
doc/lessons/019.pdf: docs/lessons/019/main.tex \
		docs/lessons/assets/019-ultrasonic-range-pencil.png \
		docs/lessons/assets/019-range-state-pencil.png
doc/lessons/020.pdf: docs/lessons/020/main.tex \
		docs/lessons/assets/020-motor-intent-pencil.png
doc/lessons/021.pdf: docs/lessons/021/main.tex \
		docs/lessons/assets/021-rover-layout-pencil.png \
		docs/lessons/assets/021-rover-layout.tex
doc/lessons/022.pdf: docs/lessons/022/main.tex \
		docs/lessons/assets/022-owned-buses-pencil.png \
		docs/lessons/assets/022-owned-buses-pencil.tex \
		docs/lessons/assets/022-record-journey-pencil.png \
		docs/lessons/assets/022-record-journey-pencil.tex
doc/lessons/023.pdf: docs/lessons/023/main.tex \
		docs/lessons/assets/023-inert-load-pencil.png \
		docs/lessons/assets/023-inert-load-pencil.svg \
		docs/lessons/assets/023-progress-pencil.png \
		docs/lessons/assets/023-progress-pencil.svg
doc/lessons/024.pdf: docs/lessons/024/main.tex \
		docs/lessons/assets/024-greenhouse-pencil.png \
		docs/lessons/assets/024-greenhouse-pencil.tex \
		docs/lessons/assets/024-lcd-closeup.png \
		docs/lessons/assets/024-lcd-closeup.tex \
		docs/lessons/assets/024-greenhouse-progress.png \
		docs/lessons/assets/024-greenhouse-progress.tex
doc/lessons/025.pdf: docs/lessons/025/main.tex \
		docs/lessons/assets/025-infrared-literal-pencil.tex \
		docs/lessons/assets/025-infrared-literal-pencil.png \
		docs/lessons/assets/025-infrared-progress-pencil.tex \
		docs/lessons/assets/025-infrared-progress-pencil.png
doc/lessons/026.pdf: docs/lessons/026/main.tex \
		docs/lessons/assets/026-telemetry-packet-pencil.png \
		docs/lessons/assets/026-telemetry-packet-pencil.tex
doc/lessons/027.pdf: docs/lessons/027/main.tex \
		docs/lessons/assets/027-telemetry-console-pencil.png \
		docs/lessons/assets/027-evidence-chain-pencil.png
doc/lessons/028.pdf: docs/lessons/028/main.tex \
		docs/lessons/assets/028-inert-channel-pencil.png \
		docs/lessons/assets/028-inert-channel-pencil.tex
doc/lessons/029.pdf: docs/lessons/029/main.tex \
		docs/lessons/assets/029-inert-cue-pencil.png \
		docs/lessons/assets/029-inert-cue-pencil.tex
doc/lessons/030.pdf: docs/lessons/030/main.tex \
		docs/lessons/assets/030-inert-show-pencil.png \
		docs/lessons/assets/030-inert-show-pencil.tex \
		docs/lessons/assets/030-process-pencil.png \
		docs/lessons/assets/030-process-pencil.tex
doc/lessons/031.pdf: docs/lessons/031/main.tex \
		docs/lessons/assets/031-analog-joystick-pencil.png
doc/lessons/032.pdf: docs/lessons/032/main.tex \
		docs/lessons/assets/032-gray-code-pencil.png
doc/lessons/033.pdf: docs/lessons/033/main.tex \
		docs/lessons/assets/033-console-orientation-pencil.png \
		docs/lessons/assets/033-evidence-chain-pencil.png
doc/lessons/034.pdf: docs/lessons/034/main.tex \
		docs/lessons/assets/034-magnetic-observation-pencil.svg \
		docs/lessons/assets/034-magnetic-observation-pencil.png \
		docs/lessons/assets/034-magnetic-transition-pencil.svg \
		docs/lessons/assets/034-magnetic-transition-pencil.png
doc/lessons/035.pdf: docs/lessons/035/main.tex \
		docs/lessons/assets/035-passage-stages-pencil.svg \
		docs/lessons/assets/035-passage-stages-pencil.png \
		docs/lessons/assets/035-passage-timing-pencil.svg \
		docs/lessons/assets/035-passage-timing-pencil.png
doc/lessons/036.pdf: docs/lessons/036/main.tex \
		docs/lessons/assets/036-commit-journey-pencil.svg \
		docs/lessons/assets/036-commit-journey-pencil.png \
		docs/lessons/assets/036-ledger-recovery-pencil.svg \
		docs/lessons/assets/036-ledger-recovery-pencil.png
doc/lessons/037.pdf: docs/lessons/037/main.tex \
		docs/lessons/assets/037-contact-stages-pencil.svg \
		docs/lessons/assets/037-contact-stages-pencil.png \
		docs/lessons/assets/037-trace-lab-pencil.svg \
		docs/lessons/assets/037-trace-lab-pencil.png
doc/lessons/038.pdf: docs/lessons/038/main.tex \
		docs/lessons/assets/038-confidence-pencil.svg \
		docs/lessons/assets/038-confidence-pencil.png \
		docs/lessons/assets/038-envelope-trace-pencil.svg \
		docs/lessons/assets/038-envelope-trace-pencil.png
doc/lessons/039.pdf: docs/lessons/039/main.tex \
		docs/lessons/assets/039-pattern-pencil.svg \
		docs/lessons/assets/039-pattern-pencil.png
doc/lessons/040.pdf: docs/lessons/040/main.tex \
		docs/lessons/assets/040-optical-traces-pencil.png
doc/lessons/041.pdf: docs/lessons/041/main.tex \
		docs/lessons/assets/041-evidence-lanes-pencil.png \
		docs/lessons/assets/041-passage-experiment-pencil.png
doc/lessons/042.pdf: docs/lessons/042/main.tex \
		docs/lessons/assets/042-course-layout-pencil.png \
		docs/lessons/assets/042-run-timeline-pencil.png
doc/lessons/043.pdf: docs/lessons/043/main.tex \
		docs/lessons/assets/043-inertial-provenance-pencil.png \
		docs/lessons/assets/043-inertial-timeline-pencil.png
doc/lessons/044.pdf: docs/lessons/044/main.tex \
		docs/lessons/assets/044-board-frame-gravity-pencil.png \
		docs/lessons/assets/044-orientation-poses-pencil.png \
		docs/lessons/assets/044-presentation-intent-pencil.png \
		docs/lessons/assets/044-replay-worksheet-pencil.png
doc/lessons/045.pdf: docs/lessons/045/main.tex \
		docs/lessons/assets/045-live-frozen-pencil.png \
		docs/lessons/assets/045-state-flow-pencil.png \
		docs/lessons/assets/045-tabletop-replay-pencil.png
doc/lessons/046.pdf: docs/lessons/046/main.tex \
		docs/lessons/046/046-copied-intent-pencil.png \
		docs/lessons/046/046-qualification-hysteresis-pencil.png
doc/lessons/047.pdf: docs/lessons/047/main.tex \
		docs/lessons/assets/047-bounded-motion-pencil.png \
		docs/lessons/assets/047-logical-phase-wheel-pencil.png
doc/lessons/048.pdf: docs/lessons/048/main.tex \
		docs/lessons/assets/048-staged-authorization-pencil.png \
		docs/lessons/assets/048-stop-flow-pencil.png
doc/lessons/049.pdf: docs/lessons/049/main.tex \
		docs/lessons/assets/049-fixed-record-anatomy-pencil.png \
		docs/lessons/assets/049-lockout-timeline-pencil.png \
		docs/lessons/assets/049-token-bin-map-pencil.png
doc/lessons/050.pdf: docs/lessons/050/main.tex \
		docs/lessons/assets/050-logical-bounds-pencil.png \
		docs/lessons/assets/050-release-acquire-pencil.png
doc/lessons/051.pdf: docs/lessons/051/main.tex \
		docs/lessons/assets/051-audit-pairs-pencil.png \
		docs/lessons/assets/051-invariant-path-pencil.png \
		docs/lessons/assets/051-joint-preview-pencil.png
doc/lessons/055.pdf: docs/lessons/055/main.tex \
		docs/lessons/assets/055-clue-wall-pencil.png \
		docs/lessons/assets/055-freshness-pencil.png \
		docs/lessons/assets/055-provenance-pencil.png \
		docs/lessons/assets/055-rule-dag-pencil.png \
		docs/lessons/assets/055-troubleshooting-pencil.png
doc/lessons/056.pdf: docs/lessons/056/main.tex \
		docs/lessons/assets/051-joint-preview-pencil.png
doc/lessons/057.pdf: docs/lessons/057/main.tex \
		docs/lessons/assets/057-atomic-solve-pencil.png \
		docs/lessons/assets/057-six-station-console-pencil.png
doc/lessons/058.pdf: docs/lessons/058/main.tex \
		docs/lessons/assets/058-diagnosis-tree-pencil.png \
		docs/lessons/assets/058-frame-swap-pencil.png \
		docs/lessons/assets/058-refresh-deadline-pencil.png \
		docs/lessons/assets/058-scanbook-pencil.png \
		docs/lessons/assets/058-three-stage-refresh-pencil.png
doc/lessons/059.pdf: docs/lessons/059/main.tex \
		docs/lessons/assets/059-dark-start-pencil.png \
		docs/lessons/assets/059-fault-ledger-pencil.png \
		docs/lessons/assets/059-orientation-pencil.png \
		docs/lessons/assets/059-recording-seam-pencil.png \
		docs/lessons/assets/059-register-envelope-pencil.png
doc/lessons/060.pdf: docs/lessons/060/main.tex \
		docs/lessons/assets/060-bench-gates-pencil.png \
		docs/lessons/assets/060-control-envelope-pencil.png \
		docs/lessons/assets/060-disagreement-pencil.png \
		docs/lessons/assets/060-generation-gate-pencil.png \
		docs/lessons/assets/060-lap-hold-pencil.png \
		docs/lessons/assets/060-one-story-pencil.png \
		docs/lessons/assets/060-perimeter-dial-pencil.png \
		docs/lessons/assets/060-receipt-ledger-pencil.png \
		docs/lessons/assets/060-self-test-pencil.png \
		docs/lessons/assets/060-stopwatch-states-pencil.png
doc/lessons/061.pdf: docs/lessons/061/main.tex \
		docs/lessons/assets/061-calibration-slopes-pencil.png \
		docs/lessons/assets/061-duty-window-pencil.png \
		docs/lessons/assets/061-evidence-boundary-pencil.png \
		docs/lessons/assets/061-freshness-rollover-pencil.png \
		docs/lessons/assets/061-quality-precedence-pencil.png \
		docs/lessons/assets/061-replay-ledger-pencil.png \
		docs/lessons/assets/061-sample-card-pencil.png \
		docs/lessons/assets/061-validation-gate-pencil.png
doc/lessons/062.pdf: docs/lessons/062/main.tex \
		docs/lessons/assets/062-evidence-boundary-pencil.png \
		docs/lessons/assets/062-independent-ages-pencil.png \
		docs/lessons/assets/062-radiant-timeline-pencil.png \
		docs/lessons/assets/062-replay-ledger-pencil.png \
		docs/lessons/assets/062-thermal-agreement-pencil.png \
		docs/lessons/assets/062-three-source-envelope-pencil.png \
		docs/lessons/assets/062-uncertainty-intervals-pencil.png \
		docs/lessons/assets/062-validation-gates-pencil.png
doc/lessons/063.pdf: docs/lessons/063/main.tex \
		docs/lessons/assets/063-alarm-cooldown-pencil.png \
		docs/lessons/assets/063-audit-collision-pencil.png \
		docs/lessons/assets/063-audit-witness-pencil.png \
		docs/lessons/assets/063-composition-boundary-pencil.png \
		docs/lessons/assets/063-hazard-frame-pencil.png \
		docs/lessons/assets/063-inert-output-intent-pencil.png \
		docs/lessons/assets/063-precedence-ladder-pencil.png \
		docs/lessons/assets/063-replay-ledger-pencil.png

$(LESSON_PDFS): | $(BUILD_MARKER)
	mkdir -p "$(BUILD_DIR)/lessons/$(basename $(notdir $@))" doc/lessons
	SOURCE_DATE_EPOCH=1785160800 \
		$(PDFLATEX) -halt-on-error -interaction=nonstopmode \
		-output-directory="$(BUILD_DIR)/lessons/$(basename $(notdir $@))" \
		"docs/lessons/$(basename $(notdir $@))/main.tex"
	SOURCE_DATE_EPOCH=1785160800 \
		$(PDFLATEX) -halt-on-error -interaction=nonstopmode \
		-output-directory="$(BUILD_DIR)/lessons/$(basename $(notdir $@))" \
		"docs/lessons/$(basename $(notdir $@))/main.tex"
	cp "$(BUILD_DIR)/lessons/$(basename $(notdir $@))/main.pdf" "$@"

lessons-check: lessons
	@for pdf in $(LESSON_PDFS); do \
		test "$$(stat -c %s "$$pdf")" -lt 50000000 || exit 1; \
		pdfinfo "$$pdf" | grep -q '^Pages:' || exit 1; \
	done
	@echo "ADK lesson PDF checks passed."

include mk/pdf_monochrome.mk
include mk/pdf_policy.mk

lessons-check: pdf-monochrome-check
lessons-check: pdf-policy-check
