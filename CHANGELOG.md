# Changelog

## [1.1.0] — 2026-06-01 🐝

First public release.

### ✨ Added
- 6 fasting programs: 12:12, 14:10, 16:8, 18:6, 20:4, and OMAD
- Circular progress ring with HH:MM:SS countdown during active fast and eating window
- **OVERTIME counter** — holds state past target and counts up until you explicitly stop
- OMAD mode with elapsed time and thin progress bar toward 23-hour target
- In-app and wakeup-relaunch **notifications** — vibration + banner when fast or eating window target is reached
- Edit-start-time screen — nudge fast start by ±15 minutes
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
