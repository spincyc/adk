LESSON_LOGS := $(addprefix $(BUILD_DIR)/lessons/,\
	$(addsuffix /main.log,$(LESSONS)))

.PHONY: pdf-policy-check lesson-visual-policy-check
lesson-visual-policy-check:
	python scripts/check_lesson_visual_policy.py \
		$(addprefix docs/lessons/,$(addsuffix /main.tex,$(LESSONS)))

$(BUILD_DIR)/lessons/%/main.log: docs/lessons/%/main.tex | $(BUILD_MARKER)
	mkdir -p "$(dir $@)"
	SOURCE_DATE_EPOCH=1785160800 \
		$(PDFLATEX) -halt-on-error -interaction=nonstopmode \
		-output-directory="$(dir $@)" "$<"
	SOURCE_DATE_EPOCH=1785160800 \
		$(PDFLATEX) -halt-on-error -interaction=nonstopmode \
		-output-directory="$(dir $@)" "$<"

pdf-policy-check: $(LESSON_PDFS) $(LESSON_LOGS)
	python scripts/check_pdf_policy.py \
		--build-directory "$(BUILD_DIR)/lessons" \
		$(LESSON_PDFS)

lessons-check: lesson-visual-policy-check
