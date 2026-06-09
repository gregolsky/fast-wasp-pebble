#!/usr/bin/env bash
set -euo pipefail

PHONE_IP="${1:-}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE="gregolsky/pebble-sdk:pebble-tool-5.0.37-sdk-4.9.169"

if [[ -z "$PHONE_IP" ]]; then
    echo "Usage: ./deploy.sh <phone-ip>" >&2
    exit 1
fi

cd "$SCRIPT_DIR"
echo "Building and installing on $PHONE_IP..."
docker run --rm \
    --network host \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    "$IMAGE" \
    bash -c "pebble build && pebble install --phone '$PHONE_IP'"
