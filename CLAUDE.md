# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Critical: build only, never flash

Per `AGENTS.md` in each module: **agents build only — never flash or monitor hardware.**
Use `idf.py build`. `idf.py flash monitor` is for the human user, not you.

Do **not** add `AGENTS.md` to `README.md` or any user-facing docs — it is dev-reference only.

## Repository shape

This is a superproject of **four git submodules**, each an independent ESP-IDF (native C, no
Arduino) firmware project or shared library. Each has its own git remote — a change inside a
submodule is committed *in that submodule*, then the superproject commit bumps the pointer.

| Path | Submodule remote | What it is |
| --- | --- | --- |
| `controller/`   | `scoreboard-clock-controller` | Master timing node: menus, sport selection, timer, TX |
| `play_clock/`   | `...-led-display-esp-ws2815` | 2-digit WS2815 seconds display: RX only, pure display |
| `repeater/`     | `...-repeater-nRF24L01` | RF range extender: RX→re-TX |
| `radio-common/` | `...-radio-common-nrf24` | Shared bare-metal nRF24L01+ SPI driver (`RadioCommon`) + shared protocol headers |

`referee_watch/` is a **plain directory** (not yet a submodule): ESP32-C3 wrist remote sending
START/STOP + RESET to the controller via encrypted ESP-NOW on WiFi channel 6 (contract in
`radio-common/include/espnow_link.h`; build with `idf.py set-target esp32c3`). When the
`scoreboard-clock-referee-watch` GitHub repo exists, convert it to a submodule.

After cloning: `git submodule update --init --recursive` (required — modules are empty otherwise).

## Build

There is **no top-level build**. Build each module independently from its own directory:

```bash
cd controller/          # or play_clock/ or repeater/
idf.py build            # build (this is what agents run)
idf.py clean            # clean build artifacts
idf.py menuconfig       # optional SDK config
```

- Framework: ESP-IDF **v6.1**, C (not C++). Requires `IDF_PATH` set (usually via `. $IDF_PATH/export.sh`).
- `radio-common` is **not** a git-tracked component of the apps; each app's `main/CMakeLists.txt`
  compiles it by **relative source path** (`../../radio-common/src/radio_common.c`) and adds
  `../../radio-common/include` to includes. `controller` additionally lists it in
  `EXTRA_COMPONENT_DIRS`. If a build can't find `radio_common.*`, the submodule isn't checked out.
- No unit-test harness — validation is on-hardware (see each module's README / `AGENTS.md`).

## Architecture

**Protocol (the contract all modules share).** Controller broadcasts a **6-byte** frame every
**250 ms (4 Hz)** while running — **3 identical copies per tick** (burst redundancy, same sequence
byte); receivers are stateless and infer everything from the payload — there are no command bytes.
Frame = `seconds_high, seconds_low, color_r, color_g, color_b, sequence`.
Time field: 0–99 whole seconds; **255** = null/clear (display off, deep red); **256+d** (d = 0–49) =
final-countdown deciseconds — the last 5 s (running *or paused*; pause is ms-accurate) are sent as
tenths at ~10 Hz and rendered as two digits ("49" = 4.9 s). **Bit 15** flags the 10 s buzzer
(football). The play clock drives a buzzer (GPIO25) from received values: beep at 10 (flagged),
beeps 5→1, long blast at 0. Color is decided controller-side and carried in
the frame; displays just render the received RGB. Defined in `radio-common/include/radio_config.h`:
**channel-agile** — boot default 76; the controller RPD-surveys `RADIO_CHANNEL_CANDIDATES`
{76, 82, 78, 74, 49, 24} and picks the quietest (operator override via the channel menu: control
button inside the sport menu); receivers hop the same list after 2 s of silence until frames appear.
**250 kbps** (`RADIO_RF_SETUP` alias — fall back to 1 Mbps if clone modules fail), 5-byte address
`0xE7×5`, payload size **6**, CRC-enabled, **auto-ACK and auto-retransmit disabled**
(fire-and-forget broadcast; ACKs from multiple receivers would collide).

> Docs were audited and corrected against code (2026-07). If docs and code ever disagree again,
> trust `radio_config.h`: bare register-level nRF24L01+ driver over SPI — no RF24Mesh.
> **Protocol facts live in `radio-common/README.md` + `radio_config.h` only** — when editing any
> other doc, summarize and link there; never copy values (channel, rate, encoding, ACK policy).
> Copied values are how the docs drifted three times.

**`radio-common`** is a portable driver: it compiles under ESP-IDF *and* (via `#if defined(ESP_PLATFORM)…`
guards in the headers) on a host, stubbing GPIO/SPI/logging. Keep those guards intact when editing.

**Controller** (`controller/main/`) is the only complex app — a **manager-based** design wired up in
`app_main`, state passed by struct pointer (dependency injection, not globals):
- `sport_manager` — sport presets & variants (Basketball/Football/Baseball/Volleyball/Lacrosse).
- `timer_manager` — countdown state machine, decides the color per remaining seconds.
- `input_handler` — rotary encoder + control/preset buttons; short press start/stop, long press reset.
- `ui_manager` — drives the ST7735 TFT via `main/ui/ui_st7735_*` (the 1602A I2C LCD path was
  removed; ST7735 is the only display).
- `radio_comm.c` wraps `radio-common` for TX with the module's pins.

**Play clock / repeater** are thin: an RX loop over `radio-common`, a 10 s link-loss timeout, a
status LED. Play clock adds `display_driver.c` + `led_strip_encoder.c` (RMT-driven WS2815 7-segment).

## Conventions (from module `AGENTS.md`)

- Naming: `snake_case` functions/vars, `UPPER_SNAKE_CASE` constants, `PascalCase` typedefs.
- `#pragma once` headers; headers in `include/`, source in `main/`.
- Delays via `vTaskDelay(pdMS_TO_TICKS(ms))`; pins as `gpio_num_t`; per-file `static const char *TAG`.
- Keep module state in structs (thread-safety), NULL-check pointer params, bool returns for errors.

## Git

Author `linroot <nevalions@gmail.com>`, **no** co-authored-by lines. `git add <file>`, not `-A`.
Commit prefixes: feat/fix/refactor/docs/chore. When changing a submodule: commit inside it first,
then commit the updated submodule pointer in the superproject.


<!-- BEGIN BEADS INTEGRATION v:1 profile:minimal hash:ca08a54f -->
## Beads Issue Tracker

This project uses **bd (beads)** for issue tracking. Run `bd prime` to see full workflow context and commands.

### Quick Reference

```bash
bd ready              # Find available work
bd show <id>          # View issue details
bd update <id> --claim  # Claim work
bd close <id>         # Complete work
```

### Rules

- Use `bd` for ALL task tracking — do NOT use TodoWrite, TaskCreate, or markdown TODO lists
- Run `bd prime` for detailed command reference and session close protocol
- Use `bd remember` for persistent knowledge — do NOT use MEMORY.md files

## Session Completion

**Before ending a work session:** file bd issues for remaining work, run the quality gates if code changed, update issue status, then push. Work only leaves this machine when the push succeeds; if it fails, resolve it and push again. Leave a short handoff note for the next session.

```bash
git pull --rebase
bd dolt push
git push
git status  # expect "up to date with origin"
```


<!-- END BEADS INTEGRATION -->
