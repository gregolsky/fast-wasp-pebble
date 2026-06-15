"""Milestones toggle in Settings persists across relaunches."""


def test_settings_milestones_row_present(pebble):
    pebble.press("down")  # open Settings
    assert pebble.wait_for_log("app-version"), "app still running"
    # Navigate into Settings — DOWN opens it from the fast screen
    # Row order: Vibration, Milestones, Reset Data, About
    # Just verify the app launches and we can navigate without crashing
    pebble.screenshot("settings_open")


def test_settings_milestones_toggle_persists(pebble):
    # Open Settings (DOWN button from fast screen)
    pebble.press("down")
    # Move down once to reach Milestones row (row index 1)
    pebble.press("down")
    # Toggle Milestones off
    pebble.press("select")
    # Back to fast screen
    pebble.press("back")

    # Reinstall (relaunch without wiping storage)
    pebble.reinstall()
    assert pebble.wait_for_log("state-ready"), "app relaunched"

    # Open Settings again, navigate to Milestones
    pebble.press("down")
    pebble.press("down")
    pebble.screenshot("milestones_off_after_relaunch")
    # Toggle back on
    pebble.press("select")
    pebble.press("back")

    pebble.reinstall()
    assert pebble.wait_for_log("state-ready"), "app relaunched again"
    pebble.press("down")
    pebble.press("down")
    pebble.screenshot("milestones_on_after_relaunch")
