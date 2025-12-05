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
| **Controller**    | Master timing control | 1602A LCD/ST7735 TFT | Rotary encoder + button  | ✅ Implemented    |
| **Play Clock**    | Seconds display (SS)  | 2×100cm digits | Status LED + test button | ✅ Implemented    |
| **Repeater**      | Network extension     | Status LED     | Automatic                | ✅ Implemented    |
| **Sport Selector**| Sport configuration   | ST7735 TFT     | Rotary encoder + button  | ✅ Implemented    |
| **Referee Watch** | Remote control        | LCD + buttons  | Multiple buttons         | ❌ Not Implemented |

## Architecture

```
scoreboard_clock/
├── controller/       # Master timing control
├── play_clock/       # 2-digit seconds display
├── repeater/         # Network range extension
├── sport_selector/   # Sport configuration module
├── radio-common/     # Shared radio library
└── README.md         # This file
```

## Communication Protocol

- **Time-based**: Controller broadcasts time every 250ms (4Hz)
- **State inference**: Receivers detect state changes from time values
- **No explicit commands**: Simplified protocol for reliability
- **Multi-sport support**: Basketball, Football, Baseball, Volleyball, Lacrosse
- **RGB color transmission**: Dynamic color data sent with time values
- **Link quality monitoring**: Visual feedback via status LEDs
- **RF24Mesh network**: Dynamic address allocation and automatic routing

### Data Frame Structure

```
seconds_high: 1B    // High byte of seconds value
seconds_low: 1B     // Low byte of seconds value
color_r: 1B         // Red color component (0-255)
color_g: 1B         // Green color component (0-255)
color_b: 1B         // Blue color component (0-255)
sequence: 1B        // Sequence number (0-255, wraps)
```

### RGB Color System

- **Normal Operation (5+ seconds)**: Orange (255, 165, 0)
- **Urgent Countdown (5-1 seconds)**: Deep Orange-Red (255, 40, 0)
- **Timer Zero (0 seconds)**: Deep Red (255, 0, 0)
- **Null Signal (0xFF)**: Deep Red (255, 0, 0) for display clear

## Hardware Requirements

- ESP32 development boards
- ESP-IDF environment
- WS2815 LED strips (12V) for play clock displays
- nRF24L01+ radio modules
- KY-040 Rotary Encoder (controller, sport_selector)
- 1602A LCD with I2C adapter OR ST7735 TFT display (controller)
- ST7735 TFT display (sport_selector)
- Momentary push buttons

### Common Radio Pins (All Modules)
- **Radio CE**: GPIO5
- **Radio CSN**: GPIO4
- **SPI**: SCK=18, MOSI=23, MISO=19

### Controller Module
- **Status LED**: GPIO17
- **Control Button**: GPIO0
- **Rotary Encoder**: CLK=GPIO33, DT=GPIO16, SW=GPIO32
- **I2C LCD**: SDA=GPIO21, SCL=GPIO22
- **ST7735 TFT**: CS=GPIO15, DC=GPIO2, RST=GPIO4, MOSI=GPIO23, SCK=GPIO18

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
