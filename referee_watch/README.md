# Referee Watch

Wrist-worn remote for the scoreboard clock controller. Two buttons —
**START/STOP** and **RESET** — sent to the controller over **ESP-NOW**
(the ESP32's WiFi radio), with vibration feedback confirming delivery.
The nRF24 display broadcast is a separate radio and is untouched by this
link.

## How it works

- Unicast ESP-NOW frame to the controller's MAC on WiFi channel 6
  (`ESPNOW_WIFI_CHANNEL` in `radio-common/include/espnow_link.h`), encrypted
  with PMK/LMK. Channel 6 sits between the nRF24 candidate channels 24 and
  49, so the uplink never overlaps the display broadcast.
- The MAC-layer ACK drives haptics: **one pulse** = delivered, **three
  pulses** = failed after 5 retries (out of range / controller off).
- Frame carries `watch_id` + a per-press `sequence`; the controller dedupes,
  so retries can never double-toggle the clock.
- Idle = light sleep with GPIO wake on either button (~50 ms press-to-send,
  ≈0.8 mA idle → 3–4 weeks on a 500 mAh LiPo). Double fail-pattern vibration
  = battery low.

## Pairing (build-time)

1. Flash controller and watch; each logs its STA MAC at boot.
2. Put the watch MAC in the controller's `include/espnow_watches.h`
   allowlist; put the controller MAC in this repo's
   `include/watch_config.h` (`CONTROLLER_MAC`), and give each watch a unique
   `WATCH_ID`.
3. **Change `ESPNOW_PMK`/`ESPNOW_LMK`** in `espnow_link.h` per deployment.
4. Reflash. An unset (all-zero) `CONTROLLER_MAC` makes the watch halt after
   logging its own MAC — flash once "empty" just to read the MAC.

## Build

```bash
idf.py set-target esp32c3
idf.py build          # agents build only; flashing is for the human
```

## Hardware BOM (~$10/unit)

| Part | Notes |
| --- | --- |
| ESP32-C3 super mini | single chip does MCU + radio |
| 2× momentary buttons | START/STOP (GPIO3), RESET (GPIO4), active-low, internal pullups |
| 1027 coin vibration motor | via NPN (2N2222) or logic-level MOSFET on GPIO5, flyback diode across motor |
| 500 mAh 402030-class LiPo | with protection circuit |
| TP4056 charge board | USB-C variant recommended |
| Slide switch | battery cutoff for storage |
| 2× 100 kΩ resistors | battery divider LiPo+ → GPIO0 (ADC) → GND |
| 3D-printed case + strap | |

## Wiring

```
LiPo+ ──[slide switch]──┬── C3 5V/BAT (via TP4056 OUT+)
                        └──[100k]──┬──[100k]── GND
                                   └── GPIO0 (battery sense)
GPIO3 ──[button]── GND      (START/STOP)
GPIO4 ──[button]── GND      (RESET)
GPIO5 ──[1k]── NPN base; motor between 3V3 and collector, diode across motor
```

Charging: TP4056 IN from USB, OUT to battery; C3 powered from OUT+ so the
watch works while charging.
