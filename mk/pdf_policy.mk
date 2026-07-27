.PHONY: pdf-policy-check
pdf-policy-check: $(LESSON_PDFS)
	python scripts/check_pdf_policy.py \
		--build-directory "$(BUILD_DIR)/lessons" \
		$(LESSON_PDFS)
