#!/usr/bin/env bash
# Capture emulator screenshots headlessly via Docker + SDL_VIDEODRIVER=dummy.
# Output: screenshots/*.png
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PLATFORM="basalt"
SDK_PY="/opt/pebble-sdk-4.5-linux64/.env/bin/python"

mkdir -p "$SCRIPT_DIR/screenshots"

docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    -e SDL_VIDEODRIVER=dummy \
    -e SDL_AUDIODRIVER=dummy \
    rebble/pebble-sdk \
    bash -c '
set -e
PLATFORM=basalt
SDK_PY="/opt/pebble-sdk-4.5-linux64/.env/bin/python"
DRIVER="tests/e2e/driver.py"

btn() { $SDK_PY $DRIVER $PLATFORM "$1" ${2:+--hold-ms "$2"}; sleep 0.5; }
shot() { pebble screenshot --emulator $PLATFORM "screenshots/$1.png"; sleep 0.3; }

echo "==> Building..."
pebble build

echo "==> Starting emulator (headless)..."
pebble wipe --emulator $PLATFORM 2>/dev/null || true
pebble install --emulator $PLATFORM build/pebble.pbw
sleep 4

echo "==> Screenshot: picker (first launch)"
shot "01_picker"

echo "==> Select 16:8 (index 2)"
btn down; btn down; btn select
sleep 1
shot "02_ready"

echo "==> Start fast"
btn select
sleep 1
shot "03_active_fast"

echo "==> Open picker via long-press to show subtitles"
btn select 800
sleep 1
shot "04_picker_subtitles"

echo "==> Back to fast; open stats (long-press down)"
btn back
sleep 0.5
btn down 800
sleep 1
shot "05_stats"

echo ""
echo "Screenshots saved to screenshots/"
ls -lh screenshots/
'
