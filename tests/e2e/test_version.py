"""Version logged on startup matches package.json."""

import json
import os


def test_version_matches_package_json(pebble_raw):
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    expected = json.load(open(os.path.join(root, "package.json")))["version"]
    assert pebble_raw.wait_for_log(f"app-version:{expected}"), \
        f"Expected app-version:{expected} in startup logs"
