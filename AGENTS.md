# Technical Specifications

Development standards and technical specifications for the ESP32-based scoreboard timer system with RGB color support.

## System Architecture

### Module Overview

| Module            | Role                  | Radio     | Display        | Status            |
| ----------------- | --------------------- | --------- | -------------- | ----------------- |
| **Controller**    | Master timing control | nRF24L01+ | ST7735 TFT | ✅ Implemented    |
| **Play Clock**    | Seconds display (SS)  | nRF24L01+ | 2×100cm digits | ✅ Implemented    |
| **Repeater**      | Network extension     | nRF24L01+ | Status LED     | ✅ Implemented    |
| **Referee Watch** | Remote control        | nRF24L01+ | LCD + buttons  | ❌ Not Implemented |

## Radio Communication Protocol

### nRF24L01+ Configuration

- **Data Rate**: 250 kbps (fall back to 1 Mbps if clone modules fail)
- **Channel**: 76 (2.476 GHz)
- **Power Level**: 0 dBm
- **Address**: 0xE7E7E7E7E7
- **Auto-ACK**: Enabled on pipe 0
- **Payload**: Fixed 6-byte payload (no dynamic payloads)
- **CRC**: 1-byte CRC
- **Retries**: `SETUP_RETR` = 0x4F (1250µs delay, 15 retries)

### Network Topology

- **Star broadcast**: Controller transmits on a fixed address; all receivers listen on the same pipe
- **Transparent repeaters**: Same-channel, single-hop relay with sequence-based duplicate
  suppression; forwards the full 6-byte frame including RGB. Not a mesh — there is no dynamic
  address allocation, route discovery, or rerouting/failover.

### Data Frame Structure

```
seconds_high: 1B    // High byte of seconds value
seconds_low: 1B     // Low byte of seconds value
color_r: 1B         // Red color component (0-255)
color_g: 1B         // Green color component (0-255)
color_b: 1B         // Blue color component (0-255)
sequence: 1B        // Sequence number (0-255, wraps)
```

### Protocol Logic

- Controller broadcasts time data every 250ms when running
- Receivers infer state transitions from time value changes
- RGB color data transmitted alongside time for dynamic display colors
- No explicit command bytes - time changes drive all state transitions
- **Null signal**: 3 seconds after the timer reaches zero, the controller switches to broadcasting
  `seconds = 0xFF` continuously. Receivers treat this as "no active timer" and clear their displays.

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

- **Time Updates**: 250ms interval from controller (4Hz)
- **Button Detection**: Immediate with duration-based logic
- **Link Timeout**: 10 seconds detection in display modules
- **Button Debounce**: ≥20ms for referee watch

### Reliability Features

- **Link Loss Detection**: Status LED warning after 10 seconds
- **Hardware CRC**: 1-byte CRC on all radio packets (nRF24L01+)
- **Sequence Numbers**: Packet tracking and loss detection
- **Hardware Auto-Retry**: `SETUP_RETR` = 1250µs delay / 15 retries per transmission

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

- **Multi-sport timing**: Basketball, Football, Baseball, Volleyball, Lacrosse
- **Integrated sport selection**: Built-in sport configuration interface
- **Rotary encoder control**: Sport selection and time adjustment
- **Duration-based button logic**: Short press (start/stop), long press (reset)
- **LCD/ST7735 display**: Sport name, current time, and status information
- **Link quality monitoring**: Visual feedback via status LED
- **Continuous broadcasting**: 4Hz update rate, 3 identical copies per tick, when timer is running
- **RGB color transmission**: Dynamic color data sent with time values

#### Play Clock Module

- **Large LED display**: 2×100cm 7-segment digits using WS2815 strips
- **Dynamic color display**: Uses received RGB values for digit colors
- **Pure display logic**: Shows received time without local processing
- **Connection monitoring**: Status LED indicates link quality
- **Built-in testing**: Number cycling test via boot button
- **Timeout detection**: 10-second link loss detection and recovery
- **Error handling**: Hardware failure detection and visual indicators

### Build Commands

```bash
cd module_name/
idf.py build flash monitor
idf.py menuconfig         # Optional configuration
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
