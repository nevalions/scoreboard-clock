#include "../include/display_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static const char* TAG = "DISPLAY_DRIVER";

// Private function declarations
static void init_segment_mapping(PlayClockDisplay* display);
static void set_segment(PlayClockDisplay* display, uint8_t digit, segment_t segment, bool on);
static void set_segment_color(PlayClockDisplay* display, uint8_t digit, segment_t segment, uint8_t r, uint8_t g, uint8_t b);
static void clear_display(PlayClockDisplay* display);
static void update_led_strip(PlayClockDisplay* display);
static void display_digit(PlayClockDisplay* display, uint8_t digit, uint8_t value);
static void show_link_warning(PlayClockDisplay* display);

bool display_begin(PlayClockDisplay* display) {
    ESP_LOGI(TAG, "Initializing display");
    
    // Initialize structure
    memset(display, 0, sizeof(PlayClockDisplay));
    
    // Allocate LED buffer
    display->led_buffer_size = LED_COUNT * 3; // RGB per LED
    display->led_buffer = (uint8_t*)malloc(display->led_buffer_size);
    if (!display->led_buffer) {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return false;
    }
    
    // Clear buffer
    memset(display->led_buffer, 0, display->led_buffer_size);
    
    // Initialize colors
    display->color_off = (color_t){0, 0, 0};
    display->color_on = (color_t){255, 255, 255};
    display->color_warning = (color_t){255, 165, 0};  // Orange
    display->color_error = (color_t){255, 0, 0};
    
    // Initialize segment mapping
    init_segment_mapping(display);
    
    display->initialized = true;
    ESP_LOGI(TAG, "Display initialized successfully");
    return true;
}

void display_set_time(PlayClockDisplay* display, uint16_t seconds) {
    if (!display->initialized) return;
    
    ESP_LOGI(TAG, "Setting time: %d seconds", seconds);
    
    // Log the time
    display->last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void display_set_link_status(PlayClockDisplay* display, bool connected) {
    if (!display->initialized) return;
    
    display->link_status = connected;
    ESP_LOGI(TAG, "Link status: %s", connected ? "connected" : "disconnected");
}

void display_set_run_mode(PlayClockDisplay* display) {
    if (!display->initialized) return;
    
    display->current_mode = DISPLAY_MODE_RUN;
    ESP_LOGI(TAG, "Display mode: RUN");
}

void display_set_stop_mode(PlayClockDisplay* display) {
    if (!display->initialized) return;
    
    display->current_mode = DISPLAY_MODE_STOP;
    ESP_LOGI(TAG, "Display mode: STOP");
}

void display_set_reset_mode(PlayClockDisplay* display) {
    if (!display->initialized) return;
    
    display->current_mode = DISPLAY_MODE_RESET;
    ESP_LOGI(TAG, "Display mode: RESET");
}

void display_show_error(PlayClockDisplay* display) {
    if (!display->initialized) return;
    
    display->current_mode = DISPLAY_MODE_ERROR;
    ESP_LOGI(TAG, "Display mode: ERROR");
}

void display_update(PlayClockDisplay* display) {
    if (!display->initialized) return;
    
    // Mock update - just blink status LED based on link
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (current_time - display->last_update_time > 1000) {  // Update every 1 second
        ESP_LOGD(TAG, "Display update - mode: %d, link: %s", 
                 display->current_mode, display->link_status ? "up" : "down");
        display->last_update_time = current_time;
    }
}

static void init_segment_mapping(PlayClockDisplay* display) {
    // Mock segment mapping for 2-digit display
    for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
        for (int segment = 0; segment < SEGMENTS_PER_DIGIT; segment++) {
            display->segments[digit][segment].start = digit * 450 + segment * 60;  // Mock positions
            display->segments[digit][segment].count = 50;  // Mock LED count per segment
        }
    }
}

static void set_segment(PlayClockDisplay* display, uint8_t digit, segment_t segment, bool on) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Set segment: digit=%d, segment=%d, on=%d", digit, segment, on);
}

static void set_segment_color(PlayClockDisplay* display, uint8_t digit, segment_t segment, uint8_t r, uint8_t g, uint8_t b) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Set segment color: digit=%d, segment=%d, RGB=(%d,%d,%d)", digit, segment, r, g, b);
}

static void clear_display(PlayClockDisplay* display) {
    // Mock implementation - clear buffer
    if (display->led_buffer) {
        memset(display->led_buffer, 0, display->led_buffer_size);
    }
}

static void update_led_strip(PlayClockDisplay* display) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Update LED strip");
}

static void display_digit(PlayClockDisplay* display, uint8_t digit, uint8_t value) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Display digit: digit=%d, value=%d", digit, value);
}

static void show_link_warning(PlayClockDisplay* display) {
    // Mock implementation - just log
    ESP_LOGW(TAG, "Link warning displayed");
}