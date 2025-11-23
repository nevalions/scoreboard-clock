# ScoreBoard Timer System

A modular wireless scoreboard system using ESP32 microcontrollers, WS2815 LED strips, and nRF24L01+ radio modules for sports timing applications.

## System Architecture

The system consists of networked nodes that communicate wirelessly to provide synchronized timing displays:

| Module | Status | Role | Display | Interface |
|--------|--------|------|---------|-----------|
| **Controller** | 🚧 In Development | Master timing control | Status LED | Smart button |
| **Play Clock** | 🚧 In Development | Seconds display (SS) | 2×100cm digits | Receive-only |
| **Referee Watch** | 🚧 In Development | Remote control | LCD + buttons | Multiple buttons |
| **Repeater** | 🚧 In Development | Network extension | Status LED | Automatic |

## Project Structure

```
scoreboard_clock/
├── README.md                 # Project overview
├── AGENTS.md                 # Agent guidelines
├── LICENSE                   # MIT License
├── play_clock/              # 🚧 Play Clock module
├── controller/              # 🚧 Controller module  
├── repeater/                # 🚧 Repeater module
├── referee_watch/           # 🚧 Referee Watch module
└── radio-common/            # 📦 Shared radio library
```

## Communication Protocol

The system uses a simplified time-based protocol over nRF24L01+ radios:

- **Controller** broadcasts time data every 250ms when running
- **Display modules** receive-only and infer state from time changes  
- **No explicit commands** - time value changes drive all state transitions

For detailed radio specifications, see [`AGENTS.md`](./AGENTS.md#radio-communication-protocol).

## Module Details

### Implemented Modules

#### Controller (`/controller/`)
- **Purpose**: Master timing control and data transmission
- **Interface**: Smart button with press duration detection
- **Features**: Continuous time broadcasting, link quality monitoring
- **Documentation**: [`controller/README.md`](./controller/README.md)

#### Play Clock (`/play_clock/`)
- **Purpose**: 2-digit seconds display (SS format)
- **Display**: 2×100cm 7-segment digits with WS2815 LEDs
- **Features**: Connection monitoring, timeout detection, automatic recovery
- **Documentation**: [`play_clock/README.md`](./play_clock/README.md)

#### Repeater (`/repeater/`)
- **Purpose**: Network range extension through packet forwarding
- **Operation**: Transparent forwarding with status monitoring
- **Features**: Statistics tracking, simple deployment
- **Documentation**: [`repeater/README.md`](./repeater/README.md)

### Planned Modules

#### Referee Watch (`/referee_watch/`)
- **Purpose**: Handheld remote control for referees
- **Interface**: LCD display with multiple control buttons
- **Status**: UI design and power management needed

## Getting Started

### Prerequisites
- ESP32 development boards
- ESP-IDF development environment
- WS2815 LED strips with 12V power supplies
- nRF24L01+ radio modules

### Repository Setup

This project uses Git submodules:

```bash
git clone --recursive https://github.com/nevalions/scoreboard-clock.git
cd scoreboard-clock
```

If already cloned:
```bash
git submodule update --init --recursive
```

### Build and Flash

Each module builds independently:

```bash
# Play Clock
cd play_clock/ && idf.py build flash monitor

# Controller  
cd controller/ && idf.py build flash monitor

# Repeater
cd repeater/ && idf.py build flash monitor
```

Optional configuration:
```bash
idf.py menuconfig
```

### Development Workflow
1. Set up ESP-IDF environment
2. Configure hardware pins in module-specific sdkconfig
3. Build and flash individual modules
4. Test radio communication between modules
5. Deploy with repeaters for range extension

## Implementation Status

| Module | Status | Notes |
|--------|--------|-------|
| **Play Clock** | 🚧 In Development | Native C implementation |
| **Controller** | 🚧 In Development | Native C implementation |
| **Repeater** | 🚧 In Development | Native C implementation |
| **Referee Watch** | 🚧 In Development | UI design, power management |

## Documentation

- **Agent Guidelines**: [`AGENTS.md`](./AGENTS.md) - Technical specifications and coding standards
- **Module Documentation**: Individual README files in each module directory
- **Radio Protocol**: Detailed communication specifications in AGENTS.md
