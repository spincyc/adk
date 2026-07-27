.PHONY: pdf-monochrome-check
pdf-monochrome-check: $(LESSON_PDFS)
	python scripts/check_pdf_monochrome.py $(LESSON_PDFS)
