#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
xhost +local:docker 2>/dev/null || true

docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    -e DISPLAY="${DISPLAY:-:0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    rebble/pebble-sdk \
    bash -c "pebble build && pip3 install pytest --quiet && python3 -m pytest tests/e2e/ -v --platform=basalt \"\$@\""
