"""Advancing the emulator clock past the target triggers wakeup relaunch."""

import pytest


@pytest.mark.skip(reason="QEMU has no time-offset API; wakeup can't be triggered programmatically")
def test_fast_complete_wakeup(pebble):
    pebble.press("select")   # start fast
