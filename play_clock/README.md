# Play Clock Module

This module implements the Play Clock display for the scoreboard system. It displays seconds (SS) on a 2-digit 7-segment LED display and receives timing data wirelessly from the controller.

## Hardware

### Components
- **MCU**: ESP32
- **Radio**: SX1278 (868 MHz) or nRF24L01+ (2.4 GHz)
- **LED Display**: WS2815 12V LED strips forming 2 × 100cm digits
- **Power**: 12V DC with power injection at both ends

### Pin Configuration
- **LED Data Pin**: GPIO13 (RMT channel 0)
- **Radio CE**: GPIO4
- **Radio CSN**: GPIO5
- **Status LED**: GPIO2
- **SPI (for nRF24L01+)**:
  - MOSI: GPIO23
  - MISO: GPIO19
  - SCK: GPIO18

### Display Specifications
- **Digits**: 2 (for seconds display - SS format)
- **Segment Height**: ~50cm (~30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (~15 LEDs)
- **LED Type**: WS2815 12V (dual data lines for reliability)
- **LED Density**: 60 LEDs/m
- **Total LEDs**: ~900 (estimated for complete display)

## Functionality

### Operation Mode
- **Receive-only**: Listens for status broadcasts from controller
- **No local timing**: Displays only data received from controller
- **Passive display**: No user interaction required

### Communication Protocol
- **Frame Type**: 0xA1 (Status frame)
- **Data Rate**: 250kbps
- **Channel**: 100 (2.500 GHz)
- **Update Rate**: Every 100-200ms from controller

### Display Features
- **Time Display**: Shows seconds (00-99)
- **Status Indication**: Different colors for RUN/STOP/RESET states
- **Link Status**: Visual indication when controller connection is lost
- **Error Display**: Shows error pattern on hardware failure

## Build and Flash

### Using PlatformIO
```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### Using ESP-IDF
```bash
# Build the project
idf.py build

# Flash to device
idf.py flash

# Monitor serial output
idf.py monitor
```

## Network Configuration

### Radio Settings
- **Node ID**: 1 (Play Clock node)
- **Controller ID**: 0 (Master node)
- **Auto-ACK**: Enabled
- **Retries**: 15 attempts, 1500µs delay
- **CRC**: 16-bit validation
- **Address Width**: 5 bytes

### Link Monitoring
- **Timeout Detection**: 800ms without status triggers warning
- **Visual Indicator**: Middle segment blinks on link loss
- **Auto-Recovery**: Normal operation resumes when connection restored

## Dependencies

### ESP-IDF Components
- `driver/gpio` - GPIO control
- `driver/rmt` - LED strip control
- `driver/spi_master` - SPI communication for radio
- `freertos` - Task management
- `esp_log` - Logging system

### No External Libraries
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

## Development Notes

### Memory Usage
- **LED Buffer**: ~2.7KB (900 LEDs × 3 bytes)
- **Stack Size**: Configured for FreeRTOS tasks
- **Heap Usage**: Minimal, mainly for radio buffers

### Timing Constraints
- **Main Loop**: 10ms cycle time
- **LED Updates**: Async via RMT hardware
- **Radio Reception**: Event-driven, non-blocking

### Power Consumption
- **ESP32**: ~160-260mA (depending on radio activity)
- **LED Strip**: 3-6A at 12V (depending on display content)
- **Radio**: ~15mA (receive mode)