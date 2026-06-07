#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

export PATH="$HOME/.local/bin:$HOME/.local/share/uv/tools/pebble-tool/bin:$PATH"
export SDL_VIDEODRIVER=dummy

cd "$SCRIPT_DIR"
pebble build

pip3 install pytest --quiet
python3 -m pytest tests/e2e/ -v --platform=basalt "$@"
