# Scoreboard Clock Refactor Campaign — Design

Date: 2026-07-05
Status: approved (user), Phase 1 in progress
Source: full cross-repo UX/latency/failsafe analysis (findings tracked as beads; see IDs below)

## Goal

A working timer and a reliable radio channel: minimize controller→display latency,
make every node recover from radio failures, and fix operator-facing UX gaps —
validated by builds only (no lab stand available); all nodes are reflashed
**together** at the next hardware session.

## Constraints and decisions (user-approved)

- **Flash strategy:** all three node types flashed together. No mixed-fleet
  compatibility paths; clean protocol change is acceptable.
- **Link-loss UX (play clock):** digits go blank after the 10 s timeout (reuses
  the 0xFF null/clear path). No flashing stale time, no dash pattern.
- **Validation:** `idf.py build` per module, zero warnings. On-hardware checks
  deferred to a written verification checklist (deliverable of Phase 3).
- Work proceeds phase by phase; each phase is committed in the submodules,
  squash-merged to main, and pushed with a superproject pointer bump.

## Phase 1 — Radio protocol (beads: tyi, eah, x3r)

**Problem.** All nodes enable auto-ACK on the shared pipe-0 address
(`0xE7E7E7E7E7`): when the controller broadcasts, play_clock and repeater both
transmit ACKs simultaneously → RF collision → `SETUP_RETR 0x4F` fires up to
15 × 1250 µs of blocking retries per frame. The repeater additionally waits a
blind fixed `delay_ms(2)` after TX and can flip back to RX mid-retry. The
controller's 1000 ms tick and 250 ms broadcast cadence free-run independently,
so remote displays lag second changes by up to ~280 ms.

**Design.**

1. **Pure broadcast, no dynamic-ACK feature** (radio-common):
   `EN_AA = 0x00` and `SETUP_RETR = 0x00` in `radio_common_configure()`.
   With auto-ACK off on the PTX, plain `W_TX_PAYLOAD` is already
   fire-and-forget — `W_TX_PAYLOAD_NOACK`/`FEATURE.EN_DYN_ACK` are not needed
   (fewer register writes; works on clone chips). `TX_DS` still signals
   completion; `MAX_RT` becomes impossible.
   *Accepted consequence:* controller `link_good` now means "frame aired",
   not "a receiver heard me" — inherent to broadcast; LED semantics
   re-documented.
2. **Repeater TX completion** : replace `delay_ms(2)` with a bounded poll of
   `TX_DS|MAX_RT` (≤5 ms cap; with no retries TX_DS lands in ~1 ms).
   **No stagger offset**: with ACKs gone the controller frame occupies the air
   ~300 µs; re-transmitting immediately places the relay ~249 ms away from the
   controller's next frame — a deliberate 60–120 ms offset would only move it
   closer. Existing sequence dedup stays.
3. **Immediate TX on second boundary** (controller): when the timer second
   changes, rewind `radio_last_transmit` so the new value is broadcast on the
   next loop iteration (≤50 ms) instead of waiting out the 250 ms cadence.
   The forced send tracks the *timer* second (not the transmitted value) so
   the 0xFF null period does not re-trigger it every loop.

## Phase 2 — Failsafe (beads: 2ti, 0f0, 4u7, dw6, uio, utv)

1. **Runtime self-healing:** receivers, after 60 s of radio silence, read back
   `RF_CH` as a canary; on mismatch or continued silence re-run
   `radio_common_configure()`; escalate to `esp_restart()` after 3 failed
   re-inits (receivers are stateless — restart is safe). Controller: re-init
   only on N consecutive TX failures; **never** auto-restarts (would wipe a
   running game timer).
2. **Boot retry:** all nodes retry radio init 3× with 1 s backoff. Controller
   draws "RADIO INIT FAILED" on the (already initialized) TFT instead of
   silently returning from `app_main`.
3. **Play clock link loss:** blank digits on the alive→false transition.
4. **RMT refactor (play clock):** transmit only the used 330-LED range; dirty
   flag so `rmt_transmit` runs only on change (~1×/s); `led_buffer` moved to
   DMA-capable/coherent memory and the placebo `volatile buffer_check` line
   removed (properly resolves deferred bead wtl).
5. Gate test-button routines once real frames are flowing; add seconds-range
   and sequence-delta sanity checks on RX.

## Phase 3 — UX (beads: wue, uo6, hgb, 86j, 5nk, e25)

1. Variant menu made functional: `sport_manager_next/prev_variant`, a
   `SELECT_VARIANT` branch in the controller input switch, confirm honors the
   scrolled index; dead `INPUT_ACTION_TIME_ADJUST` enum removed.
2. RUN/PAUSE + radio-link glyphs in a top-corner dirty-rect on the TFT,
   redrawn immediately on start/stop.
3. Rotary: consume accumulated encoder position delta per poll (no more
   one-action-per-120 ms swallowing).
4. Play clock: default brightness capped at 150/255; test-mode white capped.
5. Chores: `SETUP_RETR` comment, RF_SETUP macro naming, status-LED pattern
   documentation, 0xFF-null doc wording.
6. **Deliverable:** hardware verification checklist covering all three phases
   (range with repeater, second-boundary sync, link-loss blanking, brownout
   recovery, menu flows).

## Out of scope

- Dedicated controller TX task (re-evaluate after Phase 1 — TX drops to
  ~300 µs, likely unnecessary; noted in bead e25).
- Stronger CRC / protocol versioning; mesh or per-node addressing.
- Any flashing or on-hardware validation (user has no lab stand).
