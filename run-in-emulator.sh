#!/usr/bin/env bash
set -euo pipefail

PLATFORM="${1:-basalt}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ "$PLATFORM" != "emery" && "$PLATFORM" != "diorite" && "$PLATFORM" != "basalt" ]]; then
    echo "Usage: $0 [basalt|diorite|emery]" >&2
    echo "Note: diorite/emery QEMU emulation is broken (I2C PMIC not emulated)." >&2
    echo "      Use basalt for emulator testing; diorite/emery build for physical device." >&2
    exit 1
fi

# Allow Docker to connect to the local X display
xhost +local:docker 2>/dev/null || true

docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    -e DISPLAY="${DISPLAY:-:0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    gregolsky/pebble-sdk:pebble-tool-5.0.37-sdk-4.9.169 \
    bash -c "pebble build && pebble install --emulator $PLATFORM --logs"
