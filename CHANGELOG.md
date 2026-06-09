# Changelog

## [1.9.0] — 2026-06-10

### 🔧 Tooling
- **Version auto-injection**: `src/c/version.h` generated from `package.json` at build time — Settings → About now always shows the correct version without manual updates
- E2E test asserts startup log matches `package.json` version

---

## [1.8.0] — 2026-06-10

### 🐛 Fixed
- **OMAD**: pressing SELECT in the Ready state now correctly logs the first meal and enters active OMAD tracking

### ✨ Changed
- **Settings shortcut**: pressing DOWN on the main fast screen opens Settings directly
- **Reset/Cancel dialog**: pressing SELECT on the Stats screen shows a confirmation dialog for cancel or full data reset

---

## [1.7.0] — 2026-06-07

### ✨ Changed
- **Settings row**: Program Picker now has a visible Settings row at the bottom (replaces the hidden long-press-SELECT gesture)

### 🔧 Tooling
- 48 unit tests (up from 38); added eating-window, format-hm, and storage-defaults coverage
- E2E harness ported off Docker to local `pebble-tool` install

---

## [1.6.0] — 2026-06-04

### 🔧 Tooling

- Version bump to ensure store submission picks up the updated SDK build

---

## [1.5.0] — 2026-06-04

### 🔧 Tooling

- Switched CI build from `rebble/pebble-sdk` Docker image to `uv`-based `pebble-tool` install (`uv tool install pebble-tool` + `pebble sdk install latest`) — the Docker image was out of date and caused a 400 on developer.repebble.com submission
- `deploy.sh` likewise drops Docker and calls the locally-installed `pebble` CLI directly
- Pebble SDK arm toolchain cached in CI (`~/.pebble-sdk`) to keep build times fast
- No functional or UI changes

---

## [1.4.0] — 2026-06-04

### 🐛 Fixed
- Stats no longer reset after every stop/start — the 64-entry history ring (770 bytes) silently failed Pebble's 256-byte per-key persist limit on every write. Replaced with four scalar aggregate counters (16 bytes total) that persist correctly.

---

## [1.3.0] — 2026-06-01

### 🐛 Fixed
- Stats screen no longer crashes the app — rewrote with a plain TextLayer (no ScrollLayer), fixed window lifecycle

---

## [1.2.0] — 2026-06-01

### 🗑️ Removed
- Edit Start Time screen (Up button was confusing — opened a blank-looking screen)

### 🔧 Changed
- **Up** now opens the Stats screen from any state (was buried under long-press Down)

---

## [1.1.0] — 2026-06-01 🐝

First public release.

### ✨ Added
- 6 fasting programs: 12:12, 14:10, 16:8, 18:6, 20:4, and OMAD
- Circular progress ring with HH:MM:SS countdown during active fast and eating window
- **OVERTIME counter** — holds state past target and counts up until you explicitly stop
- OMAD mode with elapsed time and thin progress bar toward 23-hour target
- In-app and wakeup-relaunch **notifications** — vibration + banner when fast or eating window target is reached
- Stats screen — total fasts, average length, longest fast, total overtime
- Settings — vibration toggle, two-step data reset, about
- Program Picker with scrollable list of presets
- Pixel-art wasp 🐝 menu icon (25×25, native resolution)
- Mode label above program name in the main fast view (Fasting / Eating / Ready)
- Wakeup scheduling via Pebble OS so alerts fire even when the app is closed

### 🏗️ Supported platforms
- `diorite` — Core 2 Duo (144×168 B&W)
- `emery` — Pebble Time 2 (200×228 colour)
- `basalt` — Pebble Time / Time Steel (144×168 colour)

### 🔧 Tooling
- Docker-based build via `rebble/pebble-sdk`
- `deploy.sh` — one-step build + install over local network
- GitHub Actions CI — build on every push, create release with `.pbw` on tag
