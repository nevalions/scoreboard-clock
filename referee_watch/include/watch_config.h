#pragma once

#include "driver/gpio.h"

// ============================================================================
// PER-WATCH CONFIGURATION - edit before flashing each unit
// ============================================================================

// Unique per watch (1..ESPNOW_MAX_WATCHES); also used by the controller's
// dedupe, so two watches must never share an ID
#define WATCH_ID 1

// The controller's STA MAC (printed in the controller boot log as
// "ESPNOW: controller MAC xx:xx:..."). Placeholder must be replaced -
// the watch refuses to start with the all-zero MAC
#define CONTROLLER_MAC {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

// ============================================================================
// HARDWARE (ESP32-C3 super mini)
// ============================================================================

// Buttons: active-low, internal pullups, GPIO light-sleep wake
#define BUTTON_START_STOP_PIN GPIO_NUM_3
#define BUTTON_RESET_PIN GPIO_NUM_4

// Coin vibration motor via NPN/MOSFET driver, active-high
#define VIBRO_PIN GPIO_NUM_5

// Battery voltage divider (100k/100k from LiPo+) into ADC1
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_0 // GPIO0 on C3
// 2x divider: warn below 3.55V actual (fairly empty LiPo under load)
#define BATTERY_WARN_MV 1775
