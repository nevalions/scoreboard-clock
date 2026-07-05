# Technical Specifications

Development standards and technical specifications for the ESP32-based scoreboard timer system with RGB color support.

## System Architecture

### Module Overview

| Module            | Role                  | Radio     | Display / IO     | Status            |
| ----------------- | --------------------- | --------- | ---------------- | ----------------- |
| **Controller**    | Master timing control | nRF24L01+ (TX) + WiFi ESP-NOW (watch RX) | ST7735 TFT | ✅ Implemented    |
| **Play Clock**    | Seconds display (SS)  | nRF24L01+ | 2×100cm digits + buzzer | ✅ Implemented    |
| **Repeater**      | Network extension     | nRF24L01+ | Status LED       | ✅ Implemented    |
| **Referee Watch** | Remote control (ESP32-C3) | WiFi ESP-NOW | 2 buttons + vibration | ✅ Firmware (hardware validation pending) |

## Radio Communication Protocol

> **Canonical protocol reference: `radio-common/README.md` + `radio-common/include/radio_config.h`.**
> Do not restate protocol values (channel, data rate, encoding, ACK policy) in other docs —
> summarize and link there instead. Past drift came from values copied into many files.

Summary: channel-agile fire-and-forget broadcast (auto-ACK and auto-retransmit **disabled**),
250 kbps, fixed 6-byte payload, 1-byte hardware CRC, address 0xE7×5. The controller RPD-surveys
the shared candidate list and picks the quietest channel; receivers hop the same list after 2 s
of silence until they hear frames.

### Network Topology

- **Star broadcast**: Controller transmits on a fixed address; all receivers listen on the same pipe
- **Transparent repeaters**: Same-channel, single-hop relay with sequence-based duplicate
  suppression; forwards the full 6-byte frame including RGB. Not a mesh — there is no dynamic
  address allocation, route discovery, or rerouting/failover.

### Data Frame Structure

```
time_high: 1B       // Time field high byte (bit 15 = WARN10 flag)
time_low: 1B        // Time field low byte
color_r: 1B         // Red color component (0-255)
color_g: 1B         // Green color component (0-255)
color_b: 1B         // Blue color component (0-255)
sequence: 1B        // Sequence number (0-255, wraps)
```

Time-field encoding (full definition in `radio_config.h`): 0–99 whole seconds; 255 null/clear;
256+d deciseconds for the final 5 s (tenths, ~10 Hz); bit 15 flags the 10 s buzzer (football).

### Protocol Logic

- Controller broadcasts every 250ms while running, 3 identical copies per tick (burst
  redundancy); forced immediate send on every value change (~10 Hz during tenths)
- Receivers are stateless: they infer everything from the received value (no command bytes)
- **Null signal**: 3 seconds after zero the controller broadcasts 255 continuously;
  receivers clear their displays
- Play clock drives referee buzzer patterns from received values (10 s warning on WARN10
  frames, per-second beeps 5→1, long blast at 0) — downward crossings only, so duplicates
  and resets never sound

### Referee Watch Uplink (ESP-NOW)

Separate radio path: encrypted ESP-NOW on WiFi channel 6 (between nRF24 candidates 24/49),
contract in `radio-common/include/espnow_link.h`. Build-time MAC pairing, per-watch sequence
dedupe, commands join the controller's normal input-action path.

### Color Logic Implementation

Colors are per-sport (`controller/main/colors.c`), not universal:

- **Football**: Orange (255, 90, 0) normally → Deep Orange-Red (255, 40, 0) below 5s → Red (255, 0, 0) at 0/null
- **Basketball**: Always Red (255, 0, 0)
- **Baseball / Volleyball / Lacrosse**: Always Orange

### Sport Configuration Support

- **Basketball**: 24s, 30s shot clock variations
- **Football**: 40s, 25s play clock variations
- **Baseball**: 14s, 15s, 19s, 20s pitch clock variations
- **Volleyball**: 8s serve timing
- **Lacrosse**: 30s shot clock timing

## Hardware Specifications

### LED Display Standards

- **LED Type**: WS2815 12V (dual data lines DI & BI for reliability)
- **LED Density**: 60 LEDs/m (16.6mm spacing)
- **Power**: 12V DC injected at both ends of each digit
- **Format**: 7-segment display

#### Play Clock (SS Format)

- **Digit Height**: 50cm (≈30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (≈15 LEDs)
- **Total Digits**: 2

### Pin Assignments

#### Common Radio Pins (All Modules)

- **Radio CE**: GPIO5
- **Radio CSN**: GPIO4
- **SPI**: SCK=18, MOSI=23, MISO=19

#### Controller Module

- **Status LED**: GPIO2 (external link quality indicator)
- **Control Button**: GPIO0 (start/stop/reset)
- **Rotary Encoder**: CLK=GPIO33, DT=GPIO16, SW=GPIO32
- **ST7735 TFT**: CS=GPIO27, DC=GPIO26, RST=GPIO25, SDA=GPIO13, SCL=GPIO14
- **Preset Buttons (1-4)**: GPIO21, GPIO22, GPIO36, GPIO34
- **Start Button**: GPIO35
- **Reset Button**: GPIO15
- Note: GPIO34/35/36 are input-only pins and require external pull-up resistors

#### Play Clock Module

- **Status LED**: GPIO2 (built-in)
- **LED Strip Data**: GPIO13 (WS2815)
- **Test Button**: GPIO0 (boot button)

## Development Standards

### Code Requirements

- **Framework**: ESP-IDF (native C, no Arduino imports)
- **Build System**: CMake
- **Language**: C (not C++) for maximum reliability
- **Implemented Modules**: Controller, Play Clock, Repeater

### Timing Constraints

- **Time Updates**: 250ms interval from controller (4Hz); ~10 Hz during the final 5 s (tenths)
- **Timer**: millisecond-accurate; pause preserves the exact remaining fraction
- **Button Detection**: Immediate with duration-based logic
- **Link Timeout**: 10 seconds blanking in display modules; channel scan starts after 2 s silence
- **Button Debounce**: 40ms on the referee watch

### Reliability Features

- **Link Loss Detection**: Status LED warning + display blanking after 10 seconds
- **Hardware CRC**: 1-byte CRC on all radio packets; implausible values rejected in software
- **Sequence Numbers**: Packet tracking, loss logging, repeater duplicate suppression
- **Burst redundancy**: 3 identical copies per tick (auto-ACK/auto-retry are disabled — broadcast)
- **Self-healing**: receivers re-configure after 60 s silence (esp_restart after 3 failures);
  controller re-configures after 20 consecutive TX failures but never restarts (timer must survive)
- **Channel agility**: receivers re-acquire a changed channel by scanning the candidate list

### Power Management

- **LED Displays**: 12V power injection at both ends
- **WS2815**: Backup data line for reliability
- **Referee Watch**: Low power mode for battery operation
- **Repeaters**: USB or external 5V power

### Module Structure

```
module_name/
├── main/
│   ├── main.c           # Main application logic
│   ├── radio_comm.c     # Radio communication layer
│   └── display_driver.c # LED control (display modules only)
├── include/
│   ├── radio_comm.h     # Radio interface definitions
│   └── display_driver.h # LED driver interface (display modules)
├── CMakeLists.txt       # Build configuration
└── README.md           # Module-specific documentation
```

### Implemented Features

#### Controller Module

- **Multi-sport timing**: Basketball, Football, Baseball, Volleyball, Lacrosse (variant menus)
- **ms-accurate timer**: pause preserves the exact tenth; TFT shows tenths in the final 5 s
- **Rotary gestures**: running = rotation opens sport menu, click cycles TX brightness
  (day/dusk/night RGB scaling); paused = rotation adjusts time ±1s; control button in the
  sport menu opens the radio channel menu (RPD noise bars + manual pick)
- **Duration-based button logic**: Short press (start/stop), long press (reset)
- **ST7735 display**: sport, big clock, RUN/PAUSE + brightness + link status row
- **Continuous broadcasting**: 4Hz, 3 identical copies per tick; boot-time channel auto-pick
- **Referee watch uplink**: encrypted ESP-NOW receiver on the otherwise idle WiFi radio

#### Play Clock Module

- **Large LED display**: 2×100cm 7-segment digits using WS2815 strips
- **Dynamic color display**: received RGB through a gamma-2.2 LUT, brightness capped 150/255
- **Pure display logic**: renders received values incl. deciseconds ("49" = 4.9 s)
- **Referee buzzer** (GPIO25): 10 s warning (flagged frames), beeps 5→1, long blast at 0
- **Connection monitoring**: status LED patterns + 10 s link-loss blanking; channel scan
- **Built-in testing**: number cycling test via boot button (only when no radio link)

### Build Commands

```bash
cd module_name/
idf.py build              # agents build only - flash/monitor is for the human
idf.py menuconfig         # Optional configuration
# referee_watch only: idf.py set-target esp32c3 first
```

### Git Configuration

- **Author**: All commits must be attributed to "linroot"
- **Prohibited**: Never use "opencode user" in commit messages
- **Submodules**: Use `git submodule update --init --recursive`

## Testing and Validation

### Radio Communication Testing

- Verify packet reception with sequence numbers
- Test link loss detection and recovery
- Validate repeater relay behavior and sequence-based duplicate suppression
- Check CRC validation on all packets
- Test RGB color transmission and reception

### Display Testing

- Verify LED segment mapping for all digits
- Test brightness levels and color consistency
- Validate power injection effectiveness
- Check display update timing
- Test RGB color accuracy and transitions

### System Integration

- Test multi-module synchronization
- Verify button response timing
- Validate timeout and recovery behavior
- Test range with and without repeaters
- Test color transitions during countdown sequences

**Note**: Do not add AGENTS.md to README.md - this file is for development reference only.
**Note**: When you need to search docs, use `context7` tools.

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

**When ending a work session**, you MUST complete ALL steps below. Work is NOT complete until `git push` succeeds.

**MANDATORY WORKFLOW:**

1. **File issues for remaining work** - Create issues for anything that needs follow-up
2. **Run quality gates** (if code changed) - Tests, linters, builds
3. **Update issue status** - Close finished work, update in-progress items
4. **PUSH TO REMOTE** - This is MANDATORY:
   ```bash
   git pull --rebase
   bd dolt push
   git push
   git status  # MUST show "up to date with origin"
   ```
5. **Clean up** - Clear stashes, prune remote branches
6. **Verify** - All changes committed AND pushed
7. **Hand off** - Provide context for next session

**CRITICAL RULES:**
- Work is NOT complete until `git push` succeeds
- NEVER stop before pushing - that leaves work stranded locally
- NEVER say "ready to push when you are" - YOU must push
- If push fails, resolve and retry until it succeeds
<!-- END BEADS INTEGRATION -->
