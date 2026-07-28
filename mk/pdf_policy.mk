.PHONY: pdf-policy-check lesson-visual-policy-check
lesson-visual-policy-check:
	python scripts/check_lesson_visual_policy.py \
		$(addprefix docs/lessons/,$(addsuffix /main.tex,$(LESSONS)))

pdf-policy-check: $(LESSON_PDFS)
	python scripts/check_pdf_policy.py \
		--build-directory "$(BUILD_DIR)/lessons" \
		$(LESSON_PDFS)

lessons-check: lesson-visual-policy-check
