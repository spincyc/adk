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
		docs/lessons/assets/006-simon-pencil.png
doc/lessons/007.pdf: docs/lessons/007/main.tex \
		docs/lessons/assets/007-analog-input-pencil.png
doc/lessons/008.pdf: docs/lessons/008/main.tex \
		docs/lessons/assets/008-sampled-signal-pencil.png
doc/lessons/009.pdf: docs/lessons/009/main.tex \
		docs/lessons/assets/009-night-light-pencil.png
doc/lessons/010.pdf: docs/lessons/010/main.tex \
		docs/lessons/assets/010-shift-register-pencil.png
doc/lessons/011.pdf: docs/lessons/011/main.tex \
		docs/lessons/assets/011-timed-traffic-pencil.png
doc/lessons/012.pdf: docs/lessons/012/main.tex \
		docs/lessons/assets/012-traffic-junction-pencil.png
doc/lessons/013.pdf: docs/lessons/013/main.tex \
		docs/lessons/assets/013-dht11-climate-pencil.png
doc/lessons/014.pdf: docs/lessons/014/main.tex \
		docs/lessons/assets/014-character-display-pencil.png
doc/lessons/015.pdf: docs/lessons/015/main.tex \
		docs/lessons/assets/015-environmental-station-pencil.png
doc/lessons/016.pdf: docs/lessons/016/main.tex \
		docs/lessons/assets/016-matrix-keypad-pencil.png
doc/lessons/017.pdf: docs/lessons/017/main.tex \
		docs/lessons/assets/017-bounded-servo-pencil.png
doc/lessons/018.pdf: docs/lessons/018/main.tex \
		docs/lessons/assets/018-access-trainer-pencil.png
doc/lessons/019.pdf: docs/lessons/019/main.tex \
		docs/lessons/assets/019-ultrasonic-range-pencil.png
doc/lessons/020.pdf: docs/lessons/020/main.tex \
		docs/lessons/assets/020-motor-intent-pencil.png

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

lessons-check: pdf-monochrome-check
