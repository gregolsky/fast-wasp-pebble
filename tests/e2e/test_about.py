"""Settings > About row is reachable and logs settings-about."""


def test_about_row_selectable(pebble):
    pebble.press("down")      # open Settings (lands on Vibration)
    pebble.press("down")      # -> Reset Data
    pebble.press("down")      # -> About
    pebble.press("select")    # select About
    assert pebble.wait_for_log("settings-about"), \
        "Expected settings-about log when About row is selected"
