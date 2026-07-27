STYLE_SOURCES := $(shell find src lessons tests -type f \
	\( -name '*.h' -o -name '*.cpp' -o -name '*.ino' \) | sort)

.PHONY: style-check
style-check:
	python scripts/check_style.py $(STYLE_SOURCES)
