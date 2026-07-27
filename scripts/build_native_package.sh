#!/bin/sh

set -eu

output_path=${1:-${NATIVE_PACKAGE:-build/adk-native.tar.gz}}
source_date_epoch=${SOURCE_DATE_EPOCH:-0}
temporary_root=$(mktemp -d)
package_root="${temporary_root}/adk-native"

cleanup()
{
  rm -rf -- "${temporary_root}"
}

trap cleanup EXIT HUP INT TERM

case ${source_date_epoch} in
  ''|*[!0-9]*)
    echo "SOURCE_DATE_EPOCH must be a non-negative integer." >&2
    exit 1
    ;;
esac

arduino_sources='
analog_input.cpp
character_display.cpp
dht11_sensor.cpp
digital_input.cpp
digital_output.cpp
matrix_keypad.cpp
mega_avr_bus_io.cpp
mega_pulse_capture_io.cpp
piezo_sounder.cpp
pwm_output.cpp
shift_register.cpp
'

is_arduino_source()
{
  candidate=$1

  for arduino_source in ${arduino_sources}
  do
    if test "${candidate}" = "${arduino_source}"
    then
      return 0
    fi
  done

  return 1
}

for arduino_source in ${arduino_sources}
do
  test -f "src/${arduino_source}" || {
    echo "Expected Arduino-bound source is missing: src/${arduino_source}" >&2
    exit 1
  }
done

mkdir -p "${package_root}/include/adk" "${package_root}/src" \
  "${package_root}/manifest"

for header in src/*.h
do
  test -f "${header}" || continue
  cp "${header}" "${package_root}/include/adk/${header##*/}"
done

: > "${package_root}/manifest/sources.txt"
for source in src/*.cpp
do
  test -f "${source}" || continue
  source_name=${source##*/}

  if is_arduino_source "${source_name}"
  then
    continue
  fi

  if grep -Eq '#[[:space:]]*include[[:space:]]*[<"]Arduino\.h[>"]' "${source}"
  then
    echo "Arduino-bound source is not in the exclusion list: ${source}" >&2
    exit 1
  fi

  cp "${source}" "${package_root}/src/${source_name}"
  printf 'src/%s\n' "${source_name}" >> \
    "${package_root}/manifest/sources.txt"
done

cp LICENSE README.md "${package_root}/"
cat > "${package_root}/manifest/README.md" <<'EOF'
# ADK native source export

This archive exports the public declaration headers and the C++17 sources
listed in `sources.txt`. The listed sources compile without Arduino headers.
The archive is not a prebuilt binary, ABI promise, system installation, CMake
package, or pkg-config package.

Arduino-bound endpoint implementations are intentionally absent. Header
presence alone is not a claim that a hardware-facing component links or works
on a native host. The demonstrated consumer is the value-only inert channel
assessor; build other compositions only after verifying their complete source
closure.
EOF

find "${package_root}" -type d -exec chmod 0755 {} \;
find "${package_root}" -type f -exec chmod 0644 {} \;

output_directory=${output_path%/*}
if test "${output_directory}" = "${output_path}"
then
  output_directory=.
fi
mkdir -p "${output_directory}"

archive_path="${temporary_root}/adk-native.tar"
if tar --help 2>/dev/null | grep -q -- '--sort'
then
  tar --sort=name \
    --mtime="@${source_date_epoch}" \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --format=ustar \
    -cf "${archive_path}" \
    -C "${temporary_root}" \
    adk-native
else
  tar -cf "${archive_path}" -C "${temporary_root}" adk-native
fi

gzip -n -c "${archive_path}" > "${output_path}"
echo "Native package written to ${output_path}."
