#!/usr/bin/env python3
"""Button press driver -- run via pebble-tool's Python."""
import sys, time, argparse

from pebble_tool.sdk.emulator import ManagedEmulatorTransport
from libpebble2.communication.transports.qemu.protocol import QemuButton
from libpebble2.communication.transports.websocket import MessageTargetPhone
from libpebble2.communication.transports.websocket.protocol import WebSocketRelayQemu

# Bitmask values for each button (from libpebble2 QemuButton protocol).
BUTTONS = {
    'back':   1,
    'up':     2,
    'select': 4,
    'down':   8,
}

QEMU_BUTTON_PROTOCOL = 8


def press(platform, button, hold_ms):
    transport = ManagedEmulatorTransport(platform)
    transport.connect()
    btn_val = BUTTONS[button]

    def send(state):
        pkt = QemuButton(state=state)
        transport.send_packet(
            WebSocketRelayQemu(protocol=QEMU_BUTTON_PROTOCOL, data=pkt.serialise()),
            target=MessageTargetPhone()
        )

    send(btn_val)
    time.sleep(hold_ms / 1000.0)
    send(0)
    if hasattr(transport, 'disconnect'):
        transport.disconnect()


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('platform')
    p.add_argument('button', choices=list(BUTTONS.keys()))
    p.add_argument('--hold-ms', type=int, default=100)
    args = p.parse_args()
    press(args.platform, args.button, args.hold_ms)
