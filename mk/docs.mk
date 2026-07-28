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

$(LESSON_PDFS): | $(BUILD_MARKER)
	mkdir -p "$(BUILD_DIR)/lessons/$(basename $(notdir $@))" doc/lessons
	SOURCE_DATE_EPOCH=1785160800 \
		$(PDFLATEX) -halt-on-error -interaction=nonstopmode \
		-output-directory="$(BUILD_DIR)/lessons/$(basename $(notdir $@))" "$<"
	SOURCE_DATE_EPOCH=1785160800 \
		$(PDFLATEX) -halt-on-error -interaction=nonstopmode \
		-output-directory="$(BUILD_DIR)/lessons/$(basename $(notdir $@))" "$<"
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
