#!/usr/bin/env bash
# Capture emulator screenshots headlessly via Docker + SDL_VIDEODRIVER=dummy.
# Usage: ./make-screenshots.sh [diorite|emery|basalt]   (default: basalt)
#
# NOTE: diorite and emery QEMU emulation is broken (no I2C PMIC). This script
# always runs the basalt emulator internally. diorite is fine — it shares the
# exact same 144x168 frame as basalt. Pass "emery" only if you want basalt-sized
# stand-ins; for true 200x228 emery shots use your physical watch instead.
set -euo pipefail

PLATFORM="${1:-basalt}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ "$PLATFORM" != "basalt" && "$PLATFORM" != "diorite" && "$PLATFORM" != "emery" ]]; then
    echo "Usage: $0 [basalt|diorite|emery]" >&2
    exit 1
fi

if [[ "$PLATFORM" == "emery" ]]; then
    echo "WARNING: emery is 200x228; basalt emulator is 144x168." >&2
    echo "         These will be mislabelled. Use physical watch for true emery shots." >&2
fi

mkdir -p "$SCRIPT_DIR/screenshots/$PLATFORM"

docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    -e SDL_VIDEODRIVER=dummy \
    -e SDL_AUDIODRIVER=dummy \
    rebble/pebble-sdk \
    bash -c "
set -e
EMU_PLATFORM=basalt
OUT_DIR=screenshots/$PLATFORM
SDK_PY=/opt/pebble-sdk-4.5-linux64/.env/bin/python
DRIVER=tests/e2e/driver.py

btn() { \$SDK_PY \$DRIVER \$EMU_PLATFORM \"\$1\" \${2:+--hold-ms \"\$2\"}; sleep 0.5; }
shot() { pebble screenshot --emulator \$EMU_PLATFORM \"\$OUT_DIR/\$1.png\"; sleep 0.3; }

echo '==> Building...'
pebble build

echo '==> Starting emulator (headless)...'
pebble wipe --emulator \$EMU_PLATFORM 2>/dev/null || true
pebble install --emulator \$EMU_PLATFORM build/pebble.pbw
sleep 4

echo '==> 01_picker (first launch)'
shot 01_picker

echo '==> Select 16:8 (index 2)'
btn down; btn down; btn select
sleep 1
shot 02_ready

echo '==> Start fast'
btn select
sleep 1
shot 03_active_fast

echo '==> Open stats (Up short)'
btn up
sleep 1
shot 04_stats

echo ''
echo \"Screenshots saved to \$OUT_DIR/\"
ls -lh \"\$OUT_DIR/\"
"

for f in "$SCRIPT_DIR/screenshots/$PLATFORM"/*.png; do
    convert "$f" -background black -alpha remove -alpha off -define png:color-type=2 "$f"
done
