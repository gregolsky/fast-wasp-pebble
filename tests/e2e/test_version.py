"""Version in package.json matches version.h (build-time generated)."""

import json
import os


def test_version_matches_package_json():
    from conftest import ROOT_DIR
    with open(os.path.join(ROOT_DIR, "package.json")) as f:
        expected = json.load(f)["version"]
    with open(os.path.join(ROOT_DIR, "src", "c", "version.h")) as f:
        content = f.read()
    assert f'"{expected}"' in content, \
        f"Expected version.h to contain \"{expected}\""
