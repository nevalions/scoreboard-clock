# AGENTS.md - Development Guidelines

Development guidance for Claude Code (claude.ai/code) working with this ESP32-based scoreboard timer system.

## Technical Architecture

### System Overview
Modular wireless scoreboard system using ESP32 microcontrollers, WS2815 LED strips, and nRF24L01+ radio modules for sports timing applications.

### Node Types & Responsibilities

| Module | Role | Radio | Display | Implementation Status |
|--------|------|-------|---------|----------------------|
| **Controller** | Master timing control | nRF24L01+ | Status LED | ✅ Complete (C) |
| **Play Clock** | Seconds display (SS) | nRF24L01+ | 2×100cm digits | ✅ Complete (C) |
| **Game Clock** | MM:SS display | nRF24L01+ | 4×60cm digits | 🚧 Planned |
| **Referee Watch** | Remote control | nRF24L01+ | LCD + buttons | 🚧 Planned |
| **Repeater** | Network extension | nRF24L01+ | Status LED | ✅ Complete (C) |

## Radio Communication Protocol

### nRF24L01+ Configuration
- **Data Rate**: 1 Mbps
- **Channel**: 76 (2.476 GHz)
- **Power Level**: 0 dBm
- **Address**: 0xE7E7E7E7E7
- **Auto-ACK**: Enabled
- **Dynamic Payloads**: ON (≤32 bytes)
- **CRC**: 16-bit validation

### Network Topology
- **RF24Mesh**: Dynamic address allocation
- **Controller**: Master node (nodeID 0)
- **Automatic route discovery** and failover
- **Transparent repeaters** for range extension

### Data Frame Structure
```
seconds_high: 1B    // High byte of seconds value
seconds_low: 1B     // Low byte of seconds value
sequence: 1B        // Sequence number (0-255, wraps)
```

**Protocol Logic**:
- Controller broadcasts time data every 250ms when running
- Receivers infer state transitions from time value changes
- No explicit command bytes - time changes drive all state transitions

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

#### Game Clock (MM:SS Format)
- **Digit Height**: 30cm (≈20 LEDs per vertical segment)
- **Horizontal Segments**: 15cm (≈10 LEDs)
- **Total Digits**: 4

### Common Pin Assignments
- **Radio CE**: GPIO5
- **Radio CSN**: GPIO4
- **SPI**: SCK=18, MOSI=23, MISO=19
- **Status LED**: GPIO2 (built-in) or GPIO17 (external)

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
- **CRC8 Validation**: Data integrity checking
- **Sequence Numbers**: Packet tracking and loss detection
- **Auto-Retry**: Up to 3 attempts for referee watch commands
- **Mesh Rerouting**: Automatic failover on node failure

### Power Management
- **LED Displays**: 12V power injection at both ends
- **WS2815**: Backup data line for reliability
- **Referee Watch**: Low power mode for battery operation
- **Repeaters**: USB or external 5V power

## Development Workflow

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
