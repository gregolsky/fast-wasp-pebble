"""Walk through every alert in schedule order and screenshot each one.

Screenshots land in tests/e2e/_artifacts/<platform>_alert_<name>.png.
Each jump lands just before the next wakeup, so none is dropped as missed."""

from test_omad import _switch_to_omad

H = 3600
PROGRAM_18_6 = 3


def _fire(pebble, at, event, name):
    pebble.jump_to_just_before(at)
    assert pebble.wait_for_log(f"alert:{event}", timeout=30), f"{name} did not fire"
    pebble.screenshot(f"{pebble.platform}_alert_{name}")
    pebble.press("back")  # dismiss


def test_every_alert_screen(pebble_raw):
    pebble = pebble_raw
    pebble.select_program(PROGRAM_18_6)
    assert pebble.wait_for_log("state-ready")
    start = pebble.emu_now()
    pebble.press("select")
    assert pebble.wait_for_log("fast-active")

    _fire(pebble, start + 9 * H,  3, "halfway")
    _fire(pebble, start + 12 * H, 5, "ketosis")
    _fire(pebble, start + 16 * H, 6, "autophagy")
    _fire(pebble, start + 17 * H, 4, "1h_left")
    _fire(pebble, start + 18 * H, 0, "fast_complete")

    stop = pebble.emu_now()
    pebble.press("select")  # stop fast -> 6h eating window
    assert pebble.wait_for_log("eat-active")
    _fire(pebble, stop + 6 * H, 1, "eat_window_closed")

    _switch_to_omad(pebble)
    meal = pebble.emu_now()
    pebble.press("select")
    assert pebble.wait_for_log("omad-active")
    _fire(pebble, meal + 23 * H, 2, "omad")
