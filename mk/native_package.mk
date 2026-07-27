NATIVE_PACKAGE ?= $(BUILD_DIR)/package/adk-native.tar.gz

.PHONY: native-package native-package-smoke

native-package:
	NATIVE_PACKAGE="$(NATIVE_PACKAGE)" \
	NATIVE_PACKAGE_ARCHIVE="$(NATIVE_PACKAGE)" \
	CXX="$(CXX)" \
	AR="$(AR)" \
	sh scripts/build_native_package.sh

native-package-smoke: native-package
	NATIVE_PACKAGE="$(NATIVE_PACKAGE)" \
	NATIVE_PACKAGE_ARCHIVE="$(NATIVE_PACKAGE)" \
	CXX="$(CXX)" \
	AR="$(AR)" \
	sh scripts/native_package_smoke.sh
