# ScoreBoard Timer System

A modular wireless scoreboard system based on ESP32 microcontrollers and WS2815 LED strips, designed for sports timing applications.

## 🏗️ System Overview

The system consists of multiple networked nodes that communicate wirelessly to provide synchronized timing displays:

### Node Types

| Module | Status | Function | Display | Control |
|--------|--------|----------|---------|---------|
| **Controller** | ✅ Complete | Master timing control | Status LED | Smart button |
| **Play Clock** | ✅ Complete | Seconds display (SS) | 2×100cm digits | Receive-only |
| **Game Clock** | 🚧 Planned | Minutes:Seconds (MM:SS) | 4×60cm digits | Receive-only |
| **Referee Watch** | 🚧 Planned | Remote control | LCD + buttons | Multiple buttons |
| **Repeater** | ✅ Complete | Network extension | Status LED | Automatic |

## 📁 Project Structure

```
scoreboard_clock/
├── README.md                 # This file
├── AGENTS.md                 # Development guidelines for Claude
├── play_clock/              # ✅ Play Clock module
├── controller/              # ✅ Controller module  
├── game_clock/              # 🚧 Game Clock module (planned)
├── referee_watch/           # 🚧 Referee Watch module (planned)
├── repeater/                # ✅ Repeater module
└── radio-common/            # 📦 Shared radio library
```

## 📡 Communication Protocol

The system uses nRF24L01+ radio modules with a simplified time-based protocol:

- **Controller** broadcasts time data every 250ms when running
- **Display modules** receive-only and infer state from time changes
- **No explicit commands** - time value changes drive all state transitions

For detailed radio specifications and protocol structure, see [`AGENTS.md`](./AGENTS.md).

## 🔧 Module Overview

### ✅ Implemented Modules

#### Play Clock (`/play_clock/`)
- **Function**: 2-digit seconds display (SS format)
- **Display**: 2×100cm 7-segment digits with WS2815 LEDs
- **Features**: Connection monitoring, timeout detection, automatic recovery
- **Documentation**: [`play_clock/README.md`](./play_clock/README.md)

#### Controller (`/controller/`)
- **Function**: Master timing control and data transmission
- **Interface**: Smart button with press duration detection
- **Features**: Continuous time broadcasting, link quality monitoring
- **Documentation**: [`controller/README.md`](./controller/README.md)

#### Repeater (`/repeater/`)
- **Function**: Network range extension through packet forwarding
- **Operation**: Transparent forwarding with status monitoring
- **Features**: Statistics tracking, simple deployment
- **Documentation**: [`repeater/README.md`](./repeater/README.md)

### 🚧 Planned Modules

#### Game Clock (`/game_clock/`)
- **Function**: 4-digit minutes:seconds display (MM:SS format)
- **Display**: 4×60cm 7-segment digits
- **Status**: LED driver adaptation needed

#### Referee Watch (`/referee_watch/`)
- **Function**: Handheld remote control for referees
- **Interface**: LCD display with multiple control buttons
- **Status**: UI design and power management needed

## 🚀 Quick Start

### Prerequisites
- ESP32 development boards
- ESP-IDF development environment
- WS2815 LED strips with 12V power supplies
- nRF24L01+ radio modules

### Repository Setup

This project uses Git submodules for shared libraries:

```bash
git clone --recursive https://github.com/nevalions/scoreboard-clock.git
cd scoreboard-clock
```

If already cloned:
```bash
git submodule update --init --recursive
```

### Build & Flash

**Play Clock**
```bash
cd play_clock/
idf.py build flash monitor
```

**Controller**
```bash
cd controller/
idf.py build flash monitor
```

**Repeater**
```bash
cd repeater/
idf.py build flash monitor
```

Optional configuration:
```bash
idf.py menuconfig
```

### Development Workflow
1. Set up ESP-IDF environment and VS Code extension
2. Configure hardware pins in module-specific sdkconfig
3. Build and flash individual modules
4. Test radio communication between modules
5. Deploy with repeaters for range extension

## 📋 Implementation Status

| Module | Status | Implementation |
|--------|--------|----------------|
| **Play Clock** | ✅ Complete | Native C, field-tested |
| **Controller** | ✅ Complete | Native C, operational |
| **Repeater** | ✅ Complete | Native C, tested |
| **Game Clock** | 🚧 Planned | LED driver adaptation |
| **Referee Watch** | 🚧 Planned | UI design, power management |

## 🔗 Documentation

- **Development Guidelines**: [`AGENTS.md`](./AGENTS.md) - Technical specifications and coding standards
- **Module Documentation**: Individual README files in each module directory
- **Radio Protocol**: Detailed communication specifications in AGENTS.md
