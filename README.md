# ScoreBoard Timer System

A modular wireless scoreboard system based on ESP32 microcontrollers and WS2815 LED strips, designed for sports timing applications.

## 🏗️ System Architecture

The system consists of multiple networked nodes that communicate wirelessly to provide synchronized timing displays:

### Node Types

| Node | Function | Radio | Display | Role |
|------|----------|-------|---------|------|
| **Controller** | Master timing control | nRF24L01+ | Status LED | Sends time data to Play Clock |
| **Play Clock** | Seconds display (SS) | nRF24L01+ | 2×100cm digits | Receive-only display |
| **Game Clock** | Minutes:Seconds (MM:SS) | nRF24L01+ | 4×60cm digits | Receive-only display |
| **Referee Watch** | Remote control | nRF24L01+ | LCD + buttons | Sends commands |
| **Repeaters** | Network extension | nRF24L01+ | Status LED | Packet forwarding |

## 📁 Project Structure

```
scoreboard_clock/
├── README.md                 # This file
├── AGENTS.md                 # Development guidance for Claude
├── play_clock/              # ✅ Play Clock module (implemented)
├── controller/              # ✅ Controller module (implemented)
├── game_clock/              # 🚧 Game Clock module (planned)
├── referee_watch/           # 🚧 Referee Watch module (planned)
└── repeater/                # ✅ Repeater module (implemented)
```

## 📡 Radio Communication

### Common Settings (nRF24L01+)
- **Data Rate**: 1 Mbps
- **Channel**: 76 (2.476 GHz)
- **Power Level**: 0 dBm
- **Address**: 0xE7E7E7E7E7
- **Auto-ACK**: Enabled
- **Dynamic Payloads**: ON (≤32 bytes)
- **CRC**: 16-bit validation

### Protocol Structure

**Time Data Frame (Controller → Play Clock)**
```
seconds_high: 1B    // High byte of seconds value
seconds_low: 1B     // Low byte of seconds value  
sequence: 1B        // Sequence number (0-255, wraps)
```

**Note**: The controller uses a simplified time-only protocol where the receiver infers start/stop/reset commands from time value changes.

## 🔧 Module Details

### ✅ Play Clock Module (`/play_clock/`)

**Status**: Implemented and ready for deployment
**Function**: Displays seconds (SS) on 2 × 100cm digits
**Hardware**: ESP32 + WS2815 LED strips + nRF24L01+ module
**Implementation**: Native C (not C++) for maximum reliability

#### Key Features
- **Pure display operation** - Shows received time data without local logic
- **2-digit 7-segment display** for seconds (00-99)
- **Smart connection monitoring** with status LED feedback
- **Automatic recovery** when controller reconnects
- **Timeout detection** with visual warnings after 10 seconds
- **Real-time updates** - Immediate display updates when data received

#### Hardware Specifications
- **LED Type**: WS2815 12V (dual data lines for reliability)
- **LED Density**: 60 LEDs/m (16.6mm spacing)
- **Segment Height**: ~50cm (≈30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (≈15 LEDs)
- **Power**: 12V DC injected at both ends of each digit

#### 7-Segment Layout
```
 —A—
 | |
 F B
 | |
 —G—
 | |
 E C
 | |
 —D—
```

#### Documentation
- See [`/play_clock/README.md`](./play_clock/README.md) for detailed documentation
- Includes pin configurations, build instructions, and troubleshooting

---

### 🚧 Game Clock Module (`/game_clock/`)

**Status**: Planned - Not yet implemented
**Function**: Displays minutes and seconds (MM:SS) on 4 × 60cm digits
**Hardware**: ESP32 + WS2815 LED strips + Radio module

#### Planned Features
- **4-digit 7-segment display** (MM:SS format)
- **Receive-only operation** similar to Play Clock
- **Synchronized timing** with Play Clock and Controller

#### Hardware Specifications
- **LED Type**: WS2815 12V
- **Segment Height**: ~30cm (≈20 LEDs per vertical segment)
- **Horizontal Segments**: 15cm (≈10 LEDs)
- **Power**: 12V DC injected at both ends

---

### ✅ Controller Module (`/controller/`)

**Status**: Implemented and ready for deployment
**Function**: Master timing control and time data transmitter
**Hardware**: ESP32 + nRF24L01+ + Single Control Button + Status LED

#### Key Features
- **Single Smart Button**: Press duration detection for start/stop/reset
- **Time Tracking**: Maintains internal time counter
- **Continuous Updates**: Sends time data every 250ms when running
- **Link Monitoring**: Status LED shows radio link quality
- **Enhanced Logging**: Success/failure tracking and periodic status reports

#### Hardware Specifications
- **Radio**: nRF24L01+ with CE on GPIO5, CSN on GPIO4
- **Control Button**: GPIO0 (internal pull-up, press to GND)
- **Status LED**: GPIO17 (external LED recommended)
- **SPI**: Standard ESP32 pins (SCK=18, MOSI=23, MISO=19)

#### Button Operation
- **Short Press** (< 2 seconds): Toggle start/stop
- **Long Press** (≥ 2 seconds): Reset timer to 00:00 and stop

#### Documentation
- See [`/controller/README.md`](./controller/README.md) for detailed documentation
- Includes pin configurations, build instructions, and operation guide

---

### 🚧 Referee Watch Module (`/referee_watch/`)

**Status**: Planned - Not yet implemented
**Function**: Remote control for referees
**Hardware**: ESP32 + SX1278 + LCD + Buttons

#### Planned Features
- **Compact handheld design**
- **Button controls** for Start/Stop/Reset
- **LCD display** showing current time and status
- **Haptic feedback** on command confirmation
- **Low power operation** for battery use

#### Command Protocol
- Sends command packets to Controller
- Expects ACK confirmation (80-120ms timeout)
- Automatic retry (up to 3 attempts)

---

### ✅ Repeater Module (`/repeater/`)

**Status**: Implemented and ready for deployment
**Function**: Network range extension through packet forwarding
**Hardware**: ESP32 + nRF24L01+ + Status LED

#### Key Features
- **Transparent operation** - Controller and Play Clock work unchanged
- **Packet forwarding** - Receives and immediately retransmits time data
- **Status monitoring** - Built-in LED shows network activity
- **Statistics tracking** - Logs packet counts and link status every 30 seconds
- **Simple deployment** - USB or external power, no configuration needed

#### Hardware Specifications
- **Radio**: nRF24L01+ with CE on GPIO5, CSN on GPIO4
- **Status LED**: Built-in GPIO2 LED
- **SPI**: Standard ESP32 pins (SCK=18, MOSI=23, MISO=19)
- **Power**: 5V USB or external power supply

#### Operation
- **Fast blink** (200ms): Active packet forwarding
- **Slow blink** (1000ms): Idle/no activity
- **Statistics**: Packets received/retransmitted count and link status

#### Documentation
- See [`/repeater/README.md`](./repeater/README.md) for detailed documentation
- Includes wiring diagram, placement tips, and integration guide

## 🚀 Quick Start

### Prerequisites
- ESP32 development board
- ESP-IDF development environment
- WS2815 LED strips and appropriate power supplies
- Radio modules (nRF24L01+ or SX1278)

### Building the Modules

**Play Clock Module**
```bash
cd play_clock/
idf.py build flash monitor
```

**Controller Module**
```bash
cd controller/
idf.py build flash monitor
```

**Configure Project (optional)**
```bash
idf.py menuconfig         # Open configuration menu
```

### Development Workflow
1. **Set up VS Code with ESP-IDF extension**
2. **Configure hardware pins** in sdkconfig or menuconfig
3. **Build and flash** individual modules using idf.py
4. **Test radio communication** between modules
5. **Deploy in network topology** with repeaters as needed

**Note**: Play Clock module is implemented in native C for optimal performance and reliability.

## 📋 Development Status

| Module | Status | Next Steps |
|--------|--------|------------|
| **Play Clock** | ✅ Complete (C implementation) | Field testing, integration |
| **Game Clock** | 🚧 Planned | LED driver adaptation |
| **Controller** | ✅ Complete (C implementation) | System integration testing |
| **Referee Watch** | 🚧 Planned | UI design, power management |
| **Repeater** | ✅ Complete (C implementation) | Field testing, range optimization |

## 🔗 Technical Documentation

- **Development Guidelines**: See [`AGENTS.md`](./AGENTS.md) for AI development assistance
- **Module Documentation**: Each module has its own README with detailed specifications
- **Radio Protocol**: Detailed in the Radio Communication section above
