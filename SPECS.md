# Fast Pebble — Functional Specification

> *Intermittent fasting on your wrist. With a sting.*

---

## 1. Overview

Fast Pebble is a free native app for the new Pebble devices from Core Devices. It tracks an intermittent fast, the eating window that follows it, and OMAD intervals. Everything is stored locally in watch persistent storage; no companion-phone data exchange is required.

The app is glance-first: the default screen always shows the current state of the fast (or eating window, or OMAD) with a large readable countdown.

---

## 2. Who It's For

Anyone practising **time-restricted eating** or **intermittent fasting** who owns a Pebble and wants:

- A wrist-based timer they can glance at without unlocking a phone.
- A vibration alert when the target is reached.
- A simple OMAD logger.

---

## 3. Platforms & Hardware

| Device | SDK platform | Display | Colour |
|--------|--------------|---------|--------|
| **Core 2 Duo** (successor to Pebble 2) | `diorite` | 144×168 | 1-bit B&W |
| **Pebble Time 2** (PT2) | `emery` | 200×228 | 64-colour |

| Aspect | Spec |
|--------|------|
| SDK | Pebble C SDK (Core Devices fork) |
| Input | Four hardware buttons: Back, Up, Select, Down |
| Persistent storage | Pebble `persist_*` API (≤ 4 KB total budget) |
| Wakeups | Pebble `wakeup_*` API for target-reached alerts when app is closed |
| Vibration | `vibes_*` API for feedback |

No companion phone app, no PebbleKit JS, no internet, no accounts. PT2's heart-rate sensor and additional sensors are not used in v1.

---

## 4. First-Run Experience

On first launch (no saved program), the app opens directly on the **Program Picker** (§6).

---

## 5. Navigation

| Window | How to reach it |
|--------|-----------------|
| **Main** (Ready / Active fast / Eating window / OMAD) | App launch; auto-routed by state |
| **Program Picker** | From Ready state: long-press Select |
| **Stats** | From any Main state: long-press Down |
| **Settings** | From Program Picker: long-press Select |

Back button always pops the window stack. From Main, Back exits the app (standard Pebble behaviour).

Long-press is consistently used for context-switch actions; short-press for the primary action on that screen.

---

## 6. Fasting Programs

| Program | Fast | Eating window | Description |
|---------|------|---------------|-------------|
| 12:12 | 12 h | 12 h | Beginner |
| 14:10 | 14 h | 10 h | Crescendo |
| 16:8  | 16 h | 8 h  | Leangains |
| 18:6  | 18 h | 6 h  | — |
| 20:4  | 20 h | 4 h  | Warrior |
| OMAD  | 23 h | 1 h  | One Meal/Day |

The Program Picker is a scrollable list. Up/Down to move, Select to commit. Committing saves the program as default and returns to the Main window in Ready state.

---

## 7. Main Window — Fast View

### 7.1 Ready State

- Top: program name (e.g. `16:8 Leangains`).
- Middle: large "Ready" label.
- Bottom: hint — `▶ START`.

Buttons:
- **Select (short)** — start the fast. Vibrates short-pulse.
- **Select (long)** — open Program Picker.
- **Down (long)** — open Stats.

### 7.2 Active Fast — Timer

- A **circular progress ring** filling clockwise from 12 o'clock. On Core 2 Duo: solid white arc on black. On PT2: yellow arc.
- A **HH:MM:SS countdown** in 36-pt font showing time remaining to target.
- An **"OVERTIME" badge** above the countdown once the target has passed; the counter then counts upward.
- Top: program name. Bottom: `■ STOP`.

Screen redraws every 1 s. A wakeup is scheduled at `startedAt + targetHours×3600` so the app can relaunch when closed and vibrate + show "🐝 Fast complete!" then drop into the Eating Window.

Buttons:
- **Select (short)** — stop the fast; records it in history; opens Eating Window (or Ready if no eating window).
- **Up (short)** — open Edit-Start-Time screen (§7.4).
- **Back** — exit app (wakeup remains scheduled; fast continues in background).

### 7.3 Eating Window Timer

Same ring + countdown, now counting down to the end of the eating window.

Buttons:
- **Select (short)** — "Start Fasting Now": closes the window early, immediately begins a new fast.
- **Select (long)** — "End Eating Window": closes the window without starting a fast, returns to Ready.

Wakeup fires at end of window: double-pulse vibration, modal "⏱ Eating window closed."

### 7.4 Editing the Start Time

A **±15-minute nudge** screen (no full keyboard):

- Header: current effective start time, e.g. `Started 14:23`.
- Big number: offset in minutes (default 0).
- **Up / Down** — adjust by 15 minutes.
- **Select** — commit.
- **Back** — cancel.

Validation:
- Result **cannot be in the future** — shows "Too late", refuses commit.
- Result **cannot be more than 14 days in the past** — shows "Too old", refuses commit.

### 7.5 OMAD View

When the OMAD program is active:

- Top: `OMAD`.
- Middle: elapsed time since last logged meal, `HHh MMm`.
- Below: thin horizontal progress bar toward 23-hour target.
- Bottom: `🍽 ATE`.

Buttons:
- **Select (short)** — log "I Ate My Meal": records the previous interval in history, opens a new one. Vibrates short-pulse.
- **Select (long)** — open Program Picker.

Wakeup fires at 23 h: long-pulse, modal "🐝 23h since meal."

---

## 8. Stats Screen

Reached via long-press Down from any Main state. Scrollable list:

- **Total fasts** completed.
- **Average fast length** (HH:MM).
- **Longest fast** recorded (HH:MM).
- **Total overtime** accumulated (HH:MM).

---

## 9. Settings

Reached via long-press Select from the Program Picker.

| Setting | Behaviour |
|---------|-----------|
| **Vibration** | Toggle on/off. Events still show on-screen when off. |
| **Reset data** | Two-step confirm; wipes persistent storage; returns to Program Picker. |
| **About** | App name, version. |

---

## 10. Data Model & Persistence

All data in Pebble persistent storage (`persist_*`). Time values are 32-bit Unix epoch seconds.

| Key | Type | Purpose |
|-----|------|---------|
| `K_PROGRAM_ID` | uint8 | Index 0–5 into the program table |
| `K_FAST_STARTED_AT` | int32 | Epoch seconds; 0 = no active fast |
| `K_FAST_TARGET_HOURS` | uint8 | Snapshot of program target at start |
| `K_EAT_STARTED_AT` | int32 | Epoch seconds; 0 = no active eating window |
| `K_EAT_TARGET_HOURS` | uint8 | Snapshot at start |
| `K_OMAD_LAST_MEAL_AT` | int32 | Epoch seconds; 0 = no OMAD interval open |
| `K_FAST_HISTORY` | byte array | Ring buffer: 64 entries × 8 B = 512 B |
| `K_VIBRATION_ON` | uint8 | 1 = on (default), 0 = off |
| `K_WAKEUP_ID` | int32 | Active wakeup id for cancellation |

Fast-history ring buffer wraps at 64 entries (oldest silently evicted). Total budget stays well under 4 KB.

---

## 11. Background Behaviour & Wakeups

- `start_fast()` → schedule wakeup at `startedAt + targetHours×3600`.
- `start_eating_window()` → schedule wakeup at `startedAt + targetHours×3600`.
- `log_meal()` (OMAD) → schedule wakeup at `lastMealAt + 23×3600`.
- Any state change that ends the timer → cancel the prior wakeup via stored `K_WAKEUP_ID`.

On wakeup relaunch: vibrate (long-pulse for fast-complete, double for eat-end), show modal 3 s, transition to next state. Countdown is recomputed from `time() - startedAt` on each launch — no drift.

---

## 12. Non-Goals (v1)

- Body weight tracking or any chart.
- Craving / resist log.
- Custom-hours program.
- Sync, export, accounts.
- Onboarding name entry.
- PebbleKit JS or any sensor.
- Watchface variant (this is an app).

---

## 13. Glossary

| Term | Definition |
|------|------------|
| **Fast** | A timed period of not eating, tracked from Select in Ready to the moment it ends. |
| **Eating window** | The period after a fast during which eating is permitted; starts automatically. |
| **Program** | A preset defining fasting duration and eating window. |
| **Overtime** | Time elapsed beyond the target. Shown as an upward counter once the target is passed. |
| **OMAD** | One Meal a Day — single daily-meal counter rather than a fast/eat pair. |
| **Wakeup** | A scheduled app relaunch at a future time via the Pebble `wakeup` API. |
