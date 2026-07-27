STYLE_SOURCES := $(shell find src lessons tests -type f \
	\( -name '*.h' -o -name '*.cpp' -o -name '*.ino' \) | sort)

.PHONY: format format-check style-check
format:
	$(CLANG_FORMAT) -i $(STYLE_SOURCES)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(STYLE_SOURCES)

style-check:
	python scripts/check_style.py $(STYLE_SOURCES)
