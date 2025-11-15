# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a modular scoreboard timer system designed for sports timing, based on ESP32 microcontrollers and WS2815 LED strips. The system consists of multiple wireless modules that communicate via nRF24L01 and SX1278 radio modules.

## System Architecture

The system is composed of several networked nodes:

### Node Types

1. **Controller (Master)**
   - ESP32 + SX1278 (868 MHz)
   - Maintains official time/state
   - Broadcasts status every 100-200ms
   - Handles command packets from referee watch
   - Libraries: RF24, RF24Mesh

2. **Scoreboard Play Clock Module**
   - 2 × 100cm digits displaying seconds (SS)
   - ESP32 + SX1278 receiver
   - Listens to controller broadcasts only
   - WS2815 12V LED strips, 60 LEDs/m

3. **Game Clock Module**
   - 4 × 60cm digits displaying minutes and seconds (MM:SS)
   - ESP32 + SX1278 receiver
   - WS2815 12V LED strips, 60 LEDs/m

4. **Referee Watch**
   - ESP32 + SX1278 transmitter
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
- **Data rate:** 250 kbps (best sensitivity/range)
- **Channel:** 100 (≈2.500 GHz) - above busy Wi-Fi range
- **Power level:** MAX on PA/LNA modules for controller/repeaters, medium/low on watch
- **Auto-ACK & retries:** Enabled, ARC=15 retries, ARD=1500 µs
- **Dynamic payloads:** ON (≤32 bytes)
- **CRC:** 16-bit
- **Address width:** 5 bytes

### Addressing
- Uses RF24Mesh for dynamic address allocation
- Controller is master (nodeID 0)
- Automatic route discovery and failover

## Protocol Structure

### Status Broadcast Frame (≤32 bytes)
```
type=0xA1  // status
state: 1B  // 0=STOP,1=RUN,2=RESET
seconds: 2B
ms_lowres: 2B  // 0..999/10 for tenths
seq: 1B
crc8: 1B
```

### Command Frame (Watch to Controller)
```
type=0xB1  // command
cmd: 1B    // 1=START,2=STOP,3=RESET
nonce: 2B
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
- Broadcast interval: 100-200ms from controller
- Command round-trip: typical 30-80ms with one hop
- Each hop adds only a few ms at 250kbps
- Supports 2-3 hops without significant latency
- now its ESP32 project, we dont need code or improrts for Arduino
- Led display modules does not have any logic only show data from controller (master)