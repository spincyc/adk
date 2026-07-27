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
