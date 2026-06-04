#!/usr/bin/env bash
set -euo pipefail

PHONE_IP="${1:-}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ -z "$PHONE_IP" ]]; then
    echo "Usage: ./deploy.sh <phone-ip>" >&2
    exit 1
fi

if ! command -v pebble >/dev/null 2>&1; then
    echo "error: pebble CLI not found on PATH." >&2
    echo "Install with: uv tool install pebble-tool && pebble sdk install latest" >&2
    exit 1
fi

cd "$SCRIPT_DIR"
echo "Building and installing on $PHONE_IP..."
pebble build
pebble install --phone "$PHONE_IP"
