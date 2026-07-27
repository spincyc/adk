ARCH_PACKAGES := \
	arduino-cli \
	base-devel \
	clang \
	git \
	ghostscript \
	mkdocs \
	poppler \
	python \
	texlive-basic \
	texlive-latex \
	texlive-latexextra \
	texlive-latexrecommended \
	texlive-pictures

.PHONY: bootstrap bootstrap-packages bootstrap-arduino bootstrap-permissions
bootstrap: bootstrap-arduino bootstrap-permissions

bootstrap-packages:
	sudo pacman -Syu --needed $(ARCH_PACKAGES)

bootstrap-arduino: bootstrap-packages
	$(ARDUINO_CLI) core update-index
	$(ARDUINO_CLI) core install $(ARDUINO_AVR_CORE)

bootstrap-permissions: bootstrap-packages
	@found=false; inaccessible=false; \
	for port in /dev/ttyACM* /dev/ttyUSB*; do \
		[ -e "$$port" ] || continue; \
		found=true; \
		if [ -r "$$port" ] && [ -w "$$port" ]; then \
			echo "Serial-device access is ready: $$port"; \
		else \
			echo "NOTICE: $$port is not readable and writable by this user."; \
			inaccessible=true; \
		fi; \
	done; \
	if [ "$$found" = false ]; then \
		echo "No ttyACM or ttyUSB serial device is connected; access was not tested."; \
	fi; \
	if [ "$$inaccessible" = true ]; then \
		echo "For persistent or headless serial access, run:"; \
		echo "  sudo usermod -aG uucp $$(id -un)"; \
		echo "Then log out and back in before uploading."; \
	elif ! id -nG | tr ' ' '\n' | grep -qx uucp; then \
		echo "Session access may use udev/logind ACLs; uucp is only needed"; \
		echo "for persistent or headless serial access."; \
	fi
