#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE="gregolsky/pebble-sdk:pebble-tool-5.1.2-sdk-4.33.1"

# Default to full suite; pass explicit args to override with node IDs / paths.
if [ "$#" -eq 0 ]; then
    set -- tests/e2e/
fi

docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    -e SDL_VIDEODRIVER=dummy \
    -e SDL_AUDIODRIVER=dummy \
    "$IMAGE" \
    bash -c 'pebble build && uv pip install --python "$(which python3)" pytest --quiet --cache-dir /pebble/.uv-cache && python3 -m pytest -v --platform=basalt -p no:cacheprovider "$@"' \
    -- "$@"
