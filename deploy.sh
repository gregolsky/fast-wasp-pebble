#!/usr/bin/env bash
set -euo pipefail

PHONE_IP="${1:-}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ -z "$PHONE_IP" ]]; then
    echo "Usage: ./deploy.sh <phone-ip>" >&2
    exit 1
fi

echo "Building and installing on $PHONE_IP..."
docker run --rm \
    -v "$SCRIPT_DIR:/pebble" \
    -w /pebble \
    --network host \
    rebble/pebble-sdk \
    bash -c "pebble build && pebble install --phone $PHONE_IP"
