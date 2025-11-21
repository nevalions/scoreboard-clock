# Play Clock Module

ESP32-based wireless play clock display for scoreboard systems. Shows seconds (SS) on large 7-segment LED displays and receives timing data wirelessly from a central controller.

## Overview

The Play Clock module is a receive-only display unit that:
- Displays seconds (00-99) on large LED digits
- Receives timing data wirelessly from the controller
- Shows system status through LED colors
- Operates autonomously without user interaction

## Hardware

### Components
- **MCU**: ESP32
- **Radio**: SX1278 (868 MHz) or nRF24L01+ (2.4 GHz)
- **LED Display**: WS2815 12V LED strips forming 2 × 100cm digits
- **Power**: 12V DC with power injection at both ends

### Pin Configuration
| Function | Pin | Description |
|----------|-----|-------------|
| Radio CE | GPIO5 | Radio chip enable |
| Radio CSN | GPIO4 | Radio chip select |
| Status LED | GPIO2 | System status indicator |

### Display Specifications
- **Digits**: 2 (seconds display - SS format)
- **Segment Height**: ~50cm (~30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (~15 LEDs)
- **LED Type**: WS2815 12V (dual data lines for reliability)
- **LED Density**: 60 LEDs/m
- **Total LEDs**: ~900 (estimated for complete display)

## Operation

### Display Features
- **Time Display**: Shows seconds (00-99)
- **Status Colors**: Different colors for RUN/STOP/RESET states
- **Link Status**: Visual indication when controller connection is lost
- **Error Display**: Shows error pattern on hardware failure

### Communication Protocol
- **Mock Implementation**: Currently using simulated radio communication
- **Update Rate**: Every 5 seconds (mock data generation)
- **Future**: Will implement nRF24L01+ wireless protocol

### Link Monitoring
- **Timeout Detection**: 10 seconds without status triggers warning
- **Visual Indicator**: Status LED blinks on link loss
- **Auto-Recovery**: Normal operation resumes when connection restored

## Build and Flash

See [AGENTS.md](AGENTS.md) for detailed development commands and workflow.

### Quick Start
```bash
# Activate ESP-IDF environment
idf

# Build and flash
idf.py build flash monitor

# Or build only
idf.py build
```

### Development Environment
- **Language**: Native C (not C++)
- **ESP-IDF Version**: v6.1-dev
- **Target Platform**: ESP32
- **Build System**: CMake with ESP-IDF

## Dependencies

### ESP-IDF Components
- `driver/gpio` - GPIO control
- `freertos` - Task management
- `esp_log` - Logging system

### Implementation Language
- **Native C**: Project implemented in pure C (not C++)
- **Struct-based Design**: Uses C structs instead of classes
- **ESP-IDF Framework**: Built on ESP-IDF v6.1-dev

All functionality is implemented using ESP-IDF built-in components for maximum reliability and performance.

## Troubleshooting

### Display Issues
- Check 12V power supply and connections
- Verify LED strip wiring and data polarity
- Confirm power injection at both ends of each digit

### Radio Communication
- Verify radio module connections (CE, CSN, SPI)
- Check radio module power supply
- Confirm frequency and channel match with controller

### System Errors
- Check serial monitor for error messages
- Verify ESP32 power and boot sequence
- Check for hardware conflicts (pin assignments)

## Technical Specifications

### Memory Usage
- **LED Buffer**: ~2.7KB (900 LEDs × 3 bytes, mock allocation)
- **Stack Size**: Configured for FreeRTOS tasks
- **Heap Usage**: Minimal, mainly for struct allocation
- **Code Size**: Optimized C implementation

### Timing Constraints
- **Main Loop**: 50ms cycle time
- **Mock Updates**: Every 1 second for display, 5 seconds for radio data

### Power Consumption
- **ESP32**: ~160-200mA (base operation)
- **Future LED Strip**: 3-6A at 12V (when implemented)
- **Future Radio**: ~15mA (receive mode)