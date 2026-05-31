"""Persistent-storage budget: pushing 70 fasts stays under 4 KB and wraps at 64."""

import time


def test_storage_budget(pebble):
    for _ in range(70):
        pebble.press("select")   # start fast
        time.sleep(0.1)
        pebble.press("select")   # stop fast
        time.sleep(0.1)

    assert pebble.wait_for_log("ring-count:64", timeout=60), \
        "Expected ring to cap at 64 entries"

    size_lines = [l for l in pebble._log_hist if "ring-size:" in l]
    assert size_lines, "Expected ring-size log lines"
    last_size = int(size_lines[-1].split("ring-size:")[-1].strip())
    assert last_size < 4096, f"Ring size {last_size} exceeds 4 KB budget"
