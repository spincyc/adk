#!/bin/sh

set -eu

arduino_lint=${ARDUINO_LINT:-arduino-lint}
arduino_lint_version=${ARDUINO_LINT_VERSION:-1.3.0}
lint_mode=${ARDUINO_LINT_MODE:-strict}
package_ref=${PACKAGE_REF:-HEAD}
temporary_root=$(mktemp -d)
library_root="${temporary_root}/Adk"

cleanup()
{
  rm -rf -- "${temporary_root}"
}

trap cleanup 0 HUP INT TERM

command -v "${arduino_lint}" >/dev/null || {
  echo "arduino-lint is required for a release." >&2
  exit 2
}

"${arduino_lint}" --version | grep -F "${arduino_lint_version}" >/dev/null || {
  echo "arduino-lint ${arduino_lint_version} is required." >&2
  exit 2
}

mkdir -p "${library_root}"
git archive "${package_ref}" | tar -xf - -C "${library_root}"

case "${lint_mode}" in
  strict)
    set -- \
      --compliance strict \
      --project-type library \
      "${library_root}"
    ;;
  submit|update)
    set -- \
      --compliance strict \
      --library-manager "${lint_mode}" \
      --project-type library \
      "${library_root}"
    ;;
  *)
    echo "ARDUINO_LINT_MODE must be strict, submit, or update." >&2
    exit 2
    ;;
esac

"${arduino_lint}" "$@"
