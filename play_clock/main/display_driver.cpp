#include "../include/display_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static const char* TAG = "DISPLAY_DRIVER";

PlayClockDisplay::PlayClockDisplay() :
    initialized(false),
    current_mode(DISPLAY_MODE_NORMAL),
    link_status(false),
    last_update_time(0),
    led_buffer(nullptr)
{
    // Initialize colors
    color_off = {0, 0, 0};
    color_on = {255, 255, 255};
    color_warning = {255, 165, 0};  // Orange
    color_error = {255, 0, 0};
}

PlayClockDisplay::~PlayClockDisplay() {
    if (led_buffer) {
        free(led_buffer);
        led_buffer = nullptr;
    }
}

bool PlayClockDisplay::begin() {
    ESP_LOGI(TAG, "Initializing display");
    
    // Allocate LED buffer
    led_buffer_size = LED_COUNT * 3; // RGB per LED
    led_buffer = (uint8_t*)malloc(led_buffer_size);
    if (!led_buffer) {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return false;
    }
    
    // Clear buffer
    memset(led_buffer, 0, led_buffer_size);
    
    // Initialize segment mapping
    initSegmentMapping();
    
    initialized = true;
    ESP_LOGI(TAG, "Display initialized successfully");
    return true;
}

void PlayClockDisplay::setTime(uint16_t seconds) {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Setting time: %d seconds", seconds);
    
    // Log the time
    last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void PlayClockDisplay::setLinkStatus(bool connected) {
    if (!initialized) return;
    
    link_status = connected;
    ESP_LOGI(TAG, "Link status: %s", connected ? "connected" : "disconnected");
}

void PlayClockDisplay::setRunMode() {
    if (!initialized) return;
    
    current_mode = DISPLAY_MODE_RUN;
    ESP_LOGI(TAG, "Display mode: RUN");
}

void PlayClockDisplay::setStopMode() {
    if (!initialized) return;
    
    current_mode = DISPLAY_MODE_STOP;
    ESP_LOGI(TAG, "Display mode: STOP");
}

void PlayClockDisplay::setResetMode() {
    if (!initialized) return;
    
    current_mode = DISPLAY_MODE_RESET;
    ESP_LOGI(TAG, "Display mode: RESET");
}

void PlayClockDisplay::showError() {
    if (!initialized) return;
    
    current_mode = DISPLAY_MODE_ERROR;
    ESP_LOGI(TAG, "Display mode: ERROR");
}

void PlayClockDisplay::update() {
    if (!initialized) return;
    
    // Mock update - just blink status LED based on link
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (current_time - last_update_time > 1000) {  // Update every 1 second
        ESP_LOGD(TAG, "Display update - mode: %d, link: %s", 
                 current_mode, link_status ? "up" : "down");
        last_update_time = current_time;
    }
}

void PlayClockDisplay::initSegmentMapping() {
    // Mock segment mapping for 2-digit display
    for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
        for (int segment = 0; segment < SEGMENTS_PER_DIGIT; segment++) {
            segments[digit][segment].start = digit * 450 + segment * 60;  // Mock positions
            segments[digit][segment].count = 50;  // Mock LED count per segment
        }
    }
}

void PlayClockDisplay::setSegment(uint8_t digit, segment_t segment, bool on) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Set segment: digit=%d, segment=%d, on=%d", digit, segment, on);
}

void PlayClockDisplay::setSegmentColor(uint8_t digit, segment_t segment, uint8_t r, uint8_t g, uint8_t b) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Set segment color: digit=%d, segment=%d, RGB=(%d,%d,%d)", digit, segment, r, g, b);
}

void PlayClockDisplay::clearDisplay() {
    // Mock implementation - clear buffer
    if (led_buffer) {
        memset(led_buffer, 0, led_buffer_size);
    }
}

void PlayClockDisplay::updateLEDStrip() {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Update LED strip");
}

void PlayClockDisplay::displayDigit(uint8_t digit, uint8_t value) {
    // Mock implementation - just log
    ESP_LOGD(TAG, "Display digit: digit=%d, value=%d", digit, value);
}

void PlayClockDisplay::showLinkWarning() {
    // Mock implementation - just log
    ESP_LOGW(TAG, "Link warning displayed");
}