# ScoreBoard Timer System

Modular wireless scoreboard system using ESP32 microcontrollers, WS2815 LED strips, and nRF24L01+ radio modules for sports timing applications with RGB color support.

## Quick Start

```bash
git clone --recursive https://github.com/nevalions/scoreboard-clock.git
cd scoreboard-clock
git submodule update --init --recursive

# Build any module
cd controller/ && idf.py build flash monitor
```

## System Overview

| Module            | Role                  | Display        | Interface                | Status            |
| ----------------- | --------------------- | -------------- | ------------------------ | ----------------- |
| **Controller**    | Master timing control | ST7735 TFT | Rotary encoder + button  | ✅ Implemented    |
| **Play Clock**    | Seconds display (SS)  | 2×100cm digits | Status LED + test button | ✅ Implemented    |
| **Repeater**      | Network extension     | Status LED     | Automatic                | ✅ Implemented    |
| **Referee Watch** | Remote control        | LCD + buttons  | Multiple buttons         | ❌ Not Implemented |

## Architecture

```
scoreboard_clock/
├── controller/       # Master timing control (includes sport selection)
├── play_clock/       # 2-digit seconds display
├── repeater/         # Network range extension
├── radio-common/     # Shared radio library
└── README.md         # This file
```

## Communication Protocol

- **Time-based**: Controller broadcasts time every 250ms (4Hz)
- **State inference**: Receivers detect state changes from time values
- **No explicit commands**: Simplified protocol for reliability
- **Multi-sport support**: Basketball, Football, Baseball, Volleyball, Lacrosse (integrated in controller)
- **Link quality monitoring**: Visual feedback via status LEDs
- **RGB color support**: Dynamic color data transmitted with time values

### Data Frame Structure

```
seconds_high: 1B    // High byte of seconds value
seconds_low: 1B     // Low byte of seconds value
color_r: 1B         // Red color component (0-255)
color_g: 1B         // Green color component (0-255)
color_b: 1B         // Blue color component (0-255)
sequence: 1B        // Sequence number (0-255, wraps)
```

### Timer State System

Colors are per-sport (see `controller/main/colors.c`):

- **Football**: Orange (255, 90, 0) normally, Deep Orange-Red (255, 40, 0) below 5s, Red (255, 0, 0) at 0/null
- **Basketball**: Always Red (255, 0, 0)
- **Baseball / Volleyball / Lacrosse**: Always Orange

Once the timer reaches zero, the controller keeps broadcasting for 3 seconds, then switches to the
**Null Signal**: seconds value **255** (on air: high byte `0x00`, low byte `0xFF`). Receivers treat
this as "no active timer" and clear their displays.

Frames are broadcast fire-and-forget: auto-ACK and auto-retransmit are disabled on every node,
because multiple receivers share one address and simultaneous ACKs would collide.

## Hardware Requirements

- ESP32 development boards
- ESP-IDF environment
- WS2815 LED strips (12V) for play clock displays
- nRF24L01+ radio modules
- KY-040 Rotary Encoder (controller)
- ST7735 TFT display (controller)
- Momentary push buttons

### Common Radio Pins (All Modules)
- **Radio CE**: GPIO5
- **Radio CSN**: GPIO4
- **SPI**: SCK=18, MOSI=23, MISO=19

### Controller Module
- **Status LED**: GPIO2
- **Control Button**: GPIO0
- **Rotary Encoder**: CLK=GPIO33, DT=GPIO16, SW=GPIO32
- **ST7735 TFT**: CS=GPIO27, DC=GPIO26, RST=GPIO25, SDA=GPIO13, SCL=GPIO14
- **Preset Buttons (1-4)**: GPIO21, GPIO22, GPIO36, GPIO34
- **Start Button**: GPIO35
- **Reset Button**: GPIO15
- Note: GPIO34/35/36 are input-only and require external pull-up resistors

### Play Clock Module
- **Status LED**: GPIO2 (built-in)
- **LED Strip Data**: GPIO13 (WS2815)
- **Test Button**: GPIO0 (boot button)

## Development

Each module builds independently with ESP-IDF:

```bash
cd module_name/
idf.py build flash monitor
idf.py menuconfig  # Optional configuration
```

### Submodules
This project uses git submodules for shared radio communication code:
```bash
git submodule update --init --recursive  # Required after cloning
```

## Documentation

- **Module details**: Individual README files in module directories
- **Wiring guides**: WIRING.md files in each module directory
- **Technical specifications**: AGENTS.md (development reference)
- **Radio protocol**: See Communication Protocol section above
