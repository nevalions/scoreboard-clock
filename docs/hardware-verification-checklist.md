# Hardware Verification Checklist

For the first lab-stand session after the 2026-07 refactor campaign
(radio protocol, failsafe, UX — see `docs/superpowers/specs/2026-07-05-refactor-campaign-design.md`).

**Flash all three node types together** — the on-air protocol changed
(auto-ACK disabled). Old and new firmware must not be mixed.

## Phase 1 — Radio protocol

- [ ] **Basic link**: controller → play_clock shows the countdown; colors match sport.
- [ ] **Range**: walk the play clock out to the previous maximum working distance.
      Auto-ACK retries no longer paper over marginal links, so a weak link now
      drops frames instead of retrying — verify the display still tracks
      smoothly at game distances (4 Hz + immediate-on-tick sends provide
      redundancy; occasional single-frame loss is invisible).
- [ ] **Repeater chain**: controller → repeater → play_clock (controller out of
      direct range). Verify no duplicate-flicker and no stalls; repeater stats
      log shows received ≈ retransmitted (dedup active).
- [ ] **Second-boundary sync**: eyeball controller TFT vs play clock — the big
      digits should now change within ~0.1 s of the TFT (previously up to ~0.3 s).
- [ ] **Null clear**: run any preset to zero; ~3 s later the play clock blanks.

## Phase 2 — Failsafe

- [ ] **Link-loss blanking**: power off the controller mid-countdown; after 10 s
      the play clock digits go dark (not frozen); status LED fast-blinks.
- [ ] **Link recovery**: power the controller back on; display resumes on the
      next frame.
- [ ] **Radio self-heal (receiver)**: if possible, glitch the nRF24 supply on
      the play clock (brown-out) while the ESP32 stays up; within ~60 s the log
      shows "re-configuring radio" and reception resumes. After 3 failed
      recoveries the node restarts itself.
- [ ] **Controller radio death**: disconnect the controller's nRF24 at boot →
      after 3 init retries the TFT shows "RADIO FAILED" and the timer still
      works locally. Reconnect + reboot to confirm normal start.
- [ ] **RMT/wtl race**: watch the play clock digits during normal countdown for
      any corrupted segments/colors. The old `volatile buffer_check` workaround
      was removed — if glitches reappear, reopen bead `scoreboard_clock-wtl`.
- [ ] **Test-mode gating**: pressing the play clock BOOT button while frames
      are flowing is ignored (log line); with the controller off it still runs
      the test patterns.

## Phase 3 — UX

- [ ] **Variant menu**: rotary into sport menu → click → rotate cycles the
      variants with a `>` highlight → click applies the highlighted variant
      (e.g. Basketball 30, Football 25, Baseball 20/14/19). Presets still work.
- [ ] **RUN/PAUSE glyph**: START/STOP button flips the top-left TFT glyph
      immediately (green RUN / yellow PAUSE), before the next second tick.
- [ ] **Link dot**: top-right TFT dot is green while transmitting normally;
      goes red when the nRF24 is disconnected.
- [ ] **Fast rotary spin**: flick the encoder through several detents — the
      menu selection moves by the same number of steps (drained over
      successive 50 ms polls), no lost steps.
- [ ] **Brightness**: digits noticeably capped vs before (150/255); white test
      mode does not brown-out the 12 V supply.
- [ ] **Status LED patterns** match the table in `play_clock/README.md`.

## Regression sweep

- [ ] All 5 sports × default variants count down with correct colors
      (Football orange → deep orange <5 s → red at 0; Basketball always red).
- [ ] Long soak: 30+ min continuous run, no watchdog resets, no display
      corruption, sequence gaps in play_clock debug log stay rare.
- [ ] Play clock boots ~10 s faster than pre-refactor (quick test only);
      full test still available via held BOOT button.
