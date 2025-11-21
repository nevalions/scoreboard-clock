# AGENTS.md - Development Guidelines

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a modular scoreboard timer system designed for sports timing, based on ESP32 microcontrollers and WS2815 LED strips. The system consists of multiple wireless modules that communicate via radio modules. Currently implemented: Play Clock module with ESP32 framework.

## System Architecture

The system is composed of several networked nodes:

### Node Types

1. **Controller (Master)**
   - ESP32 + nRF24L01+
   - Maintains official time/state
   - Sends commands to Play Clock on button press
   - Sends time updates every 10 seconds when running
   - 3 control buttons: START, STOP, RESET

2. **Scoreboard Play Clock Module**
   - 2 × 100cm digits displaying seconds (SS)
   - ESP32 + nRF24L01+ receiver
   - Listens to controller broadcasts only
   - WS2815 12V LED strips, 60 LEDs/m

3. **Game Clock Module**
   - 4 × 60cm digits displaying minutes and seconds (MM:SS)
   - ESP32 + nRF24L01+ receiver
   - WS2815 12V LED strips, 60 LEDs/m

4. **Referee Watch**
   - ESP32 + nRF24L01+ transmitter
   - LCD display with time and status
   - Sends START/STOP/RESET commands to controller
   - Button debouncing (≥20ms)
   - Retry mechanism: up to 3 retries if no ACK within 80-120ms

5. **Repeaters (R1/R2/R3)**
   - Mesh network nodes without application logic
   - High placement (2.5-4m) for optimal range
   - Powered from mains or LiFePO4 + buck converter

## Network Configuration

### Radio Module Settings (nRF24L01)

- **Data rate:** 1 Mbps
- **Channel:** 76 (2.476 GHz)
- **Power level:** 0 dBm
- **Auto-ACK:** Enabled
- **Dynamic payloads:** ON (≤32 bytes)
- **CRC:** 16-bit
- **Address:** 0xE7E7E7E7E7

### Addressing

- Uses RF24Mesh for dynamic address allocation
- Controller is master (nodeID 0)
- Automatic route discovery and failover

## Protocol Structure

### Command Frame (Controller to Play Clock)

```
cmd: 1B    // 0=STOP,1=RUN,2=RESET
seconds: 2B
seq: 1B
```

## Hardware Specifications

### LED Display Configuration

- **LED type:** WS2815 12V (dual data lines DI & BI)
- **Density:** 60 LEDs/m (16.6mm spacing)
- **Power:** 12V DC injected at both ends of each digit
- **7-segment display format**

#### Play Clock Digits

- **Height:** 50cm (≈30 LEDs per vertical segment)
- **Horizontal segments:** 25cm (15 LEDs)
- **Digits:** 2 (SS format)

#### Game Clock Digits

- **Height:** 30cm (≈20 LEDs per vertical segment)
- **Horizontal segments:** 15cm (10 LEDs)
- **Digits:** 4 (MM:SS format)

## Development Notes

### Power Management

- 12V power injection at both ends of each digit for voltage stability
- WS2815 provides backup data line for reliability
- Battery considerations for referee watch (low power mode)

### Communication Reliability

- Link loss detection: if no status for 800ms, blink middle segment as warning
- CRC8 validation for data integrity
- Sequence numbers for packet tracking
- Automatic mesh rerouting on node failure

### Timing Requirements

- Time update interval: 10 seconds from controller when running
- Command transmission: Immediate on button press
- ESP32 project using ESP-IDF framework (no Arduino code/imports)
- LED display modules are receive-only, displaying data from controller (master)
- Play Clock and Controller modules are fully implemented with CMake build system
