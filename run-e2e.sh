#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE="gregolsky/pebble-sdk:pebble-tool-5.0.37-sdk-4.9.169"

docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    -e SDL_VIDEODRIVER=dummy \
    -e SDL_AUDIODRIVER=dummy \
    "$IMAGE" \
    bash -c 'pebble build && uv pip install --python "$(which python3)" pytest --quiet && python3 -m pytest tests/e2e/ -v --platform=basalt "$@"' \
    -- "$@"
