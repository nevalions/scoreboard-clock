# nRF24L01+ Wiring Guide

## ESP32 to nRF24L01+ Connections

| nRF24L01+ Pin | ESP32 Pin | Function | Description |
|---------------|-----------|----------|-------------|
| VCC | 3.3V | Power | 3.3V power supply |
| GND | GND | Ground | Common ground |
| CE | GPIO5 | Chip Enable | Controls transmit/receive mode |
| CSN | GPIO4 | SPI Chip Select | SPI slave select |
| SCK | GPIO18 | SPI Clock | Serial clock |
| MOSI | GPIO23 | SPI Master Out | Data from ESP32 to nRF24L01+ |
| MISO | GPIO19 | SPI Master In | Data from nRF24L01+ to ESP32 |
| IRQ | (Optional) | Interrupt | Data available interrupt (not used in current implementation) |

## Wiring Diagram

```
ESP32              nRF24L01+
------             ----------
3.3V -------------- VCC
GND --------------- GND
GPIO5 ------------ CE
GPIO4 ------------ CSN
GPIO18 ----------- SCK
GPIO23 ----------- MOSI
GPIO19 ----------- MISO
```

## Power Requirements

- **Voltage**: 3.3V (do NOT use 5V - will damage the module)
- **Current**: Up to 15mA during transmission
- **Decoupling**: Add 10µF capacitor between VCC and GND near the nRF24L01+ module for stability

## SPI Configuration

- **SPI Host**: SPI2_HOST
- **Clock Speed**: 1 MHz
- **SPI Mode**: 0 (CPOL=0, CPHA=0)
- **Bit Order**: MSB first

## Antenna Considerations

- Keep antenna away from metal objects
- Maintain at least 20mm clearance from PCB ground planes
- For best range, use external antenna or ensure proper PCB antenna layout

## Troubleshooting

1. **No Communication**: Check VCC is 3.3V, not 5V
2. **Intermittent Connection**: Add 10µF decoupling capacitor
3. **Short Range**: Check antenna placement and power supply stability
4. **SPI Errors**: Verify all SPI connections and ensure proper grounding

## Default Configuration

- **Channel**: 76 (2.476 GHz)
- **Data Rate**: 1 Mbps
- **Power**: 0 dBm
- **CRC**: Enabled
- **Payload Size**: 32 bytes
- **Address Width**: 5 bytes
- **RX Address**: 0xE7E7E7E7E7