#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

// LED strip configuration for Play Clock (mock)
#define LED_STRIP_PIN GPIO_NUM_13
#define LED_COUNT 900  // Approximate total LEDs for 2 digits
// #define LED_RMT_CHANNEL RMT_CHANNEL_0

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

// Color structure
typedef struct {
    uint8_t r, g, b;
} color_t;

// Segment LED ranges for 2-digit play clock
typedef struct {
    uint16_t start;
    uint16_t count;
} segment_range_t;

// Play clock display structure - displays seconds (SS) only
typedef struct {
    bool initialized;
    display_mode_t current_mode;
    bool link_status;
    uint32_t last_update_time;

    // LED strip buffer
    uint8_t* led_buffer;
    size_t led_buffer_size;

    // Segment LED ranges for 2-digit play clock
    segment_range_t segments[PLAY_CLOCK_DIGITS][SEGMENTS_PER_DIGIT];

    // Color definitions
    color_t color_off;
    color_t color_on;
    color_t color_warning;
    color_t color_error;
} PlayClockDisplay;

// Function declarations
bool display_begin(PlayClockDisplay* display);
void display_set_time(PlayClockDisplay* display, uint16_t seconds);
void display_set_link_status(PlayClockDisplay* display, bool connected);
void display_set_run_mode(PlayClockDisplay* display);
void display_set_stop_mode(PlayClockDisplay* display);
void display_set_reset_mode(PlayClockDisplay* display);
void display_show_error(PlayClockDisplay* display);
void display_update(PlayClockDisplay* display);