# ScoreBoard Timer System

Modular wireless scoreboard system using ESP32 microcontrollers, WS2815 LED strips, and nRF24L01+ radio modules for sports timing applications.

## Quick Start

```bash
git clone --recursive https://github.com/nevalions/scoreboard-clock.git
cd scoreboard-clock
git submodule update --init --recursive

# Build any module
cd play_clock/ && idf.py build flash monitor
```

## System Overview

| Module | Role | Display | Interface | Status |
|--------|------|---------|-----------|--------|
| **Controller** | Master timing control | 1602A LCD | Rotary encoder + button | ✅ Implemented |
| **Play Clock** | Seconds display (SS) | 2×100cm digits | Status LED + test button | ✅ Implemented |
| **Referee Watch** | Remote control | LCD + buttons | Multiple buttons | 🚧 In Development |
| **Repeater** | Network extension | Status LED | Automatic | 🚧 In Development |

## Architecture

```
scoreboard_clock/
├── play_clock/       # 2-digit seconds display
├── controller/       # Master timing control
├── repeater/         # Network range extension
├── referee_watch/    # Handheld remote control
├── radio-common/     # Shared radio library
├── AGENTS.md         # Technical specifications
└── README.md         # This file
```

## Communication Protocol

- **Time-based**: Controller broadcasts time every 250ms
- **State inference**: Receivers detect state changes from time values
- **No explicit commands**: Simplified protocol for reliability
- **Multi-sport support**: Basketball, Football, Baseball, Volleyball, Lacrosse
- **Link quality monitoring**: Visual feedback via status LEDs

## Hardware Requirements

- ESP32 development boards
- ESP-IDF environment
- WS2815 LED strips (12V)
- nRF24L01+ radio modules
- KY-040 Rotary Encoder (controller)
- 1602A LCD with I2C adapter (controller)
- Momentary push buttons

## Development

Each module builds independently with ESP-IDF:

```bash
cd module_name/
idf.py build flash monitor
idf.py menuconfig  # Optional configuration
```

## Documentation

- **Technical specs**: [`AGENTS.md`](./AGENTS.md)
- **Module details**: Individual README files in module directories
