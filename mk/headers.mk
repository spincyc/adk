PUBLIC_HEADERS  := $(sort $(wildcard src/*.h))
HEADER_CPPFLAGS := -Isrc -Itests/fake_arduino
HEADER_CXXFLAGS := -std=c++11 -fno-exceptions -fno-rtti
HEADER_CXXFLAGS += -Wall -Wextra -Wpedantic -Wconversion -Werror

.PHONY: headers-check
headers-check:
	@set -eu; \
	for header in $(PUBLIC_HEADERS); do \
		echo "Checking $$header"; \
		"$(CXX)" $(HEADER_CPPFLAGS) $(HEADER_CXXFLAGS) \
			-x c++ -fsyntax-only -include "$$header" /dev/null; \
	done
