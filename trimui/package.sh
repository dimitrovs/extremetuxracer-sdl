#!/bin/bash
# Assemble the SD-card zip. Run inside the build container after build.sh:
#   /work/etr = this repository, /work/gl4es = gl4es checkout
set -e
OUT=/work/etr/build-trimui
STAGE="$OUT/stage"
PORT="$STAGE/Roms/PORTS"
rm -rf "$STAGE" "$OUT/ExtremeTuxRacer-trimui.zip"
mkdir -p "$PORT/ExtremeTuxRacer/licenses" "$PORT/Imgs"

aarch64-linux-gnu-strip -o "$PORT/ExtremeTuxRacer/etr" "$OUT/etr"
cp -r /work/etr/data "$PORT/ExtremeTuxRacer/data"
find "$PORT/ExtremeTuxRacer/data" -name 'Makefile*' -delete
cp /work/etr/COPYING "$PORT/ExtremeTuxRacer/licenses/COPYING.ExtremeTuxRacer-GPLv2"
cp /work/etr/AUTHORS "$PORT/ExtremeTuxRacer/licenses/AUTHORS.ExtremeTuxRacer"
cp /work/gl4es/LICENSE "$PORT/ExtremeTuxRacer/licenses/LICENSE.gl4es-MIT"
cp /work/etr/trimui/README.md "$PORT/ExtremeTuxRacer/README.md"
cp "/work/etr/trimui/Extreme Tux Racer.sh" "$PORT/"
cp "/work/etr/trimui/Extreme Tux Racer.png" "$PORT/Imgs/"

cd "$STAGE" && zip -qr "$OUT/ExtremeTuxRacer-trimui.zip" Roms
echo "packaged $OUT/ExtremeTuxRacer-trimui.zip"
