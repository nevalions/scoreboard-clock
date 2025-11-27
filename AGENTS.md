# Technical Specifications

Development standards and technical specifications for the ESP32-based scoreboard timer system with RGB color support.

## System Architecture

### Module Overview

| Module            | Role                  | Radio     | Display        | Status            |
| ----------------- | --------------------- | --------- | -------------- | ----------------- |
| **Controller**    | Master timing control | nRF24L01+ | 1602A LCD      | ✅ Implemented    |
| **Play Clock**    | Seconds display (SS)  | nRF24L01+ | 2×100cm digits | ✅ Implemented    |
| **Referee Watch** | Remote control        | nRF24L01+ | LCD + buttons  | 🚧 In Development |
| **Repeater**      | Network extension     | nRF24L01+ | Status LED     | 🚧 In Development |

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

### Color Logic Implementation

- **Normal Operation (5+ seconds)**: Orange (255, 165, 0)
- **Urgent Countdown (5-1 seconds)**: Deep Orange-Red (255, 40, 0)
- **Timer Zero (0 seconds)**: Deep Red (255, 0, 0)
- **Null Signal (0xFF)**: Deep Red (255, 0, 0) for display clear

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

- **Status LED**: GPIO17 (external link quality indicator)
- **Control Button**: GPIO0 (start/stop/reset)
- **Rotary Encoder**: CLK=GPIO34, DT=GPIO35, SW=GPIO32
- **I2C LCD**: SDA=GPIO21, SCL=GPIO22

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
- **CRC8 Validation**: Data integrity checking
- **Sequence Numbers**: Packet tracking and loss detection
- **Auto-Retry**: Up to 3 attempts for referee watch commands
- **Mesh Rerouting**: Automatic failover on node failure

### Power Management

- **LED Displays**: 12V power injection at both ends
- **WS2815**: Backup data line for reliability
- **Referee Watch**: Low power mode for battery operation
- **Repeaters**: USB or external 5V power

## Development Standards

### Code Requirements

- **Framework**: ESP-IDF (native C, no Arduino imports)
- **Build System**: CMake
- **Language**: C (not C++) for maximum reliability
- **Implemented Modules**: Controller, Play Clock, Repeater

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
- **Rotary encoder control**: Sport selection and time adjustment
- **Duration-based button logic**: Short press (start/stop), long press (reset)
- **LCD display**: Sport name, current time, and status information
- **Link quality monitoring**: Visual feedback via status LED
- **Continuous broadcasting**: 4Hz update rate when timer is running
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
- Validate mesh routing with repeaters
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
