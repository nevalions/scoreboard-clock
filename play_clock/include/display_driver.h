#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/rmt.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// LED strip configuration
#define LED_STRIP_PIN GPIO_NUM_13
#define LED_COUNT 900  // Approximate total LEDs for 2 digits
#define LED_RMT_CHANNEL RMT_CHANNEL_0

// 7-segment display configuration for Play Clock (2 digits)
#define PLAY_CLOCK_DIGITS 2
#define SEGMENTS_PER_DIGIT 7

// Play Clock dimensions: 2 × 100 cm digits
// LED segments: ~30 LEDs tall per vertical, ~15 LEDs horizontal
#define LEDS_PER_SEGMENT_VERTICAL 30
#define LEDS_PER_SEGMENT_HORIZONTAL 15

// Segment indices for 7-segment display
typedef enum {
    SEGMENT_A = 0,  // Top horizontal
    SEGMENT_B = 1,  // Upper right vertical
    SEGMENT_C = 2,  // Lower right vertical
    SEGMENT_D = 3,  // Bottom horizontal
    SEGMENT_E = 4,  // Lower left vertical
    SEGMENT_F = 5,  // Upper left vertical
    SEGMENT_G = 6   // Middle horizontal
} segment_t;

// Display modes
typedef enum {
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_STOP,
    DISPLAY_MODE_RUN,
    DISPLAY_MODE_RESET,
    DISPLAY_MODE_ERROR,
    DISPLAY_MODE_LINK_WARNING
} display_mode_t;

// Play clock display class - displays seconds (SS) only
class PlayClockDisplay {
private:
    bool initialized;
    display_mode_t current_mode;
    bool link_status;
    uint32_t last_update_time;

    // LED strip buffer
    uint8_t* led_buffer;
    size_t led_buffer_size;

    // Segment LED ranges for 2-digit play clock
    struct {
        uint16_t start;
        uint16_t count;
    } segments[PLAY_CLOCK_DIGITS][SEGMENTS_PER_DIGIT];

    // Color definitions
    struct {
        uint8_t r, g, b;
    } color_off, color_on, color_warning, color_error;

    // Private methods
    void initSegmentMapping();
    void setSegment(uint8_t digit, segment_t segment, bool on);
    void setSegmentColor(uint8_t digit, segment_t segment, uint8_t r, uint8_t g, uint8_t b);
    void clearDisplay();
    void updateLEDStrip();
    void displayDigit(uint8_t digit, uint8_t value);
    void showLinkWarning();

public:
    PlayClockDisplay();
    ~PlayClockDisplay();

    bool begin();
    void setTime(uint16_t seconds);  // Display seconds (0-99 for play clock)
    void setLinkStatus(bool connected);
    void setRunMode();
    void setStopMode();
    void setResetMode();
    void showError();
    void update();
};

#ifdef __cplusplus
}
#endif