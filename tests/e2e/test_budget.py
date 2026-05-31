"""Persistent-storage budget: pushing 70 fasts stays under 4 KB and wraps at 64."""

import time
import subprocess


def test_storage_budget(pebble):
    # Trigger 70 fast start/stop cycles rapidly via button presses.
    # In a real session we'd use a debug AppMessage to inject entries;
    # here we rely on the app logging the ring size after each push.
    for _ in range(70):
        pebble.press("select")   # start fast
        time.sleep(0.1)
        pebble.press("select")   # stop fast
        time.sleep(0.1)

    # The app logs "ring-size:<bytes>" and "ring-count:<n>" after each push.
    assert pebble.wait_for_log("ring-count:64", timeout=30), \
        "Expected ring to cap at 64 entries"

    # Also confirm the byte size logged is under 4096.
    result = subprocess.run(
        ["pebble", "logs", "--emulator", pebble.platform, "--count", "200"],
        capture_output=True, text=True
    )
    ring_size_lines = [l for l in result.stdout.splitlines() if "ring-size:" in l]
    assert ring_size_lines, "Expected ring-size log lines"
    last_size = int(ring_size_lines[-1].split("ring-size:")[-1].strip())
    assert last_size < 4096, f"Ring size {last_size} exceeds 4 KB budget"
