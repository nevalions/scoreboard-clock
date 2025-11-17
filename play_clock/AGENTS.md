# AGENTS.md - ESP32 Play Clock Development Guide

## Development Workflow

### Build Commands
```bash
# Build project
idf.py build

# Flash to device
idf.py flash

# Monitor serial output
idf.py monitor

# Build and flash in one command
idf.py build flash monitor

# Clean build
idf.py fullclean

# Configure project
idf.py menuconfig
```

### Testing
```bash
# Build all tests
idf.py build

# Run specific test (edit test file app_main() to select test)
# Edit test/test_*.cpp files to run individual tests
# Flash test firmware:
idf.py flash monitor
```

## Code Style Guidelines

### Formatting
- 4-space indentation, no tabs
- Braces on same line for functions/methods
- Maximum line length: 120 characters

### Naming Conventions
- Constants: `UPPER_SNAKE_CASE` (e.g., `LED_STRIP_PIN`, `STATUS_TIMEOUT_MS`)
- Variables: `lower_snake_case` (e.g., `system_state`, `last_status_time`)
- Classes: `PascalCase` (e.g., `PlayClockDisplay`)
- Functions: `lower_snake_case()` (e.g., `display.begin()`, `radio_init()`)
- Tags: `UPPER_SNAKE_CASE` (e.g., `TAG = "PLAY_CLOCK"`)

### Types & Headers
- Use standard C/C++ types: `uint8_t`, `uint16_t`, `bool`, `uint32_t`
- Header guards: `#pragma once`
- C++ headers: `<cstdint>`, `<stdbool.h>` for C compatibility
- ESP-IDF headers: `#include "esp_log.h"`, `#include "freertos/FreeRTOS.h"`
- Project headers: `#include "display_driver.h"`, `#include "radio_comm.h"`

### Error Handling
- ESP-IDF logging: `ESP_LOGI(TAG, "message")`, `ESP_LOGE(TAG, "error")`
- Return `bool` for success/failure in initialization functions
- Test assertions: `TEST_ASSERT_TRUE(condition)`, `TEST_ASSERT_EQUAL(expected, actual)`
- Handle hardware failures gracefully with infinite loops or error states

### ESP-IDF Specific
- Entry point: `extern "C" void app_main(void)`
- FreeRTOS delays: `vTaskDelay(pdMS_TO_TICKS(ms))`
- GPIO control: `gpio_set_direction(pin, GPIO_MODE_OUTPUT)`, `gpio_set_level(pin, level)`
- Memory: Prefer stack allocation, use `malloc()` only when necessary