#!/usr/bin/env bash
set -euo pipefail

PHONE_IP="${1:-}"

if [[ -z "$PHONE_IP" ]]; then
    echo "Usage: ./deploy.sh <phone-ip>" >&2
    exit 1
fi

echo "Building..."
pebble build

echo "Installing on $PHONE_IP..."
pebble install --phone "$PHONE_IP"
