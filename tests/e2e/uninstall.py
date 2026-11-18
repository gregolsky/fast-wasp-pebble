#!/usr/bin/env python3
"""Uninstall an app from the emulated watch, as removing it on the phone does.
Run via pebble-tool's Python."""
import sys, uuid, threading

from pebble_tool.sdk.emulator import ManagedEmulatorTransport
from libpebble2.communication import PebbleConnection
from libpebble2.services.blobdb import BlobDBClient
from libpebble2.protocol.blobdb import BlobDatabaseID, BlobStatus


def uninstall(platform, app_uuid):
    pebble = PebbleConnection(ManagedEmulatorTransport(platform))
    pebble.connect()
    pebble.run_async()
    done, result = threading.Event(), []
    BlobDBClient(pebble).delete(BlobDatabaseID.App, uuid.UUID(app_uuid),
                                lambda status: (result.append(status), done.set()))
    if not done.wait(10) or result[0] != BlobStatus.Success:
        sys.exit(f"uninstall failed: {result}")


if __name__ == '__main__':
    uninstall(sys.argv[1], sys.argv[2])
