#include "display_driver.h"
#include "esp_log.h"
#include <string.h>

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
    }
}

bool PlayClockDisplay::begin() {
    ESP_LOGI(TAG, "Initializing Play Clock Display...");

    // Initialize LED strip buffer
    led_buffer_size = LED_COUNT * 3;  // RGB bytes per LED
    led_buffer = (uint8_t*)malloc(led_buffer_size);
    if (!led_buffer) {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return false;
    }

    // Clear buffer
    memset(led_buffer, 0, led_buffer_size);

    // Initialize RMT for LED control
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_STRIP_PIN, LED_RMT_CHANNEL);
    config.clk_div = 2;  // Set counter clock to 40MHz

    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    // Initialize segment mapping for 2-digit play clock
    initSegmentMapping();

    // Clear display initially
    clearDisplay();
    updateLEDStrip();

    initialized = true;
    ESP_LOGI(TAG, "Play Clock Display initialized successfully");
    return true;
}

void PlayClockDisplay::initSegmentMapping() {
    // Map LED segments for 2-digit play clock display
    // This is a simplified mapping - adjust based on actual LED layout

    uint16_t current_led = 0;

    for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
        // Segment A (top horizontal)
        segments[digit][SEGMENT_A].start = current_led;
        segments[digit][SEGMENT_A].count = LEDS_PER_SEGMENT_HORIZONTAL;
        current_led += segments[digit][SEGMENT_A].count;

        // Segment B (upper right vertical)
        segments[digit][SEGMENT_B].start = current_led;
        segments[digit][SEGMENT_B].count = LEDS_PER_SEGMENT_VERTICAL;
        current_led += segments[digit][SEGMENT_B].count;

        // Segment C (lower right vertical)
        segments[digit][SEGMENT_C].start = current_led;
        segments[digit][SEGMENT_C].count = LEDS_PER_SEGMENT_VERTICAL;
        current_led += segments[digit][SEGMENT_C].count;

        // Segment D (bottom horizontal)
        segments[digit][SEGMENT_D].start = current_led;
        segments[digit][SEGMENT_D].count = LEDS_PER_SEGMENT_HORIZONTAL;
        current_led += segments[digit][SEGMENT_D].count;

        // Segment E (lower left vertical)
        segments[digit][SEGMENT_E].start = current_led;
        segments[digit][SEGMENT_E].count = LEDS_PER_SEGMENT_VERTICAL;
        current_led += segments[digit][SEGMENT_E].count;

        // Segment F (upper left vertical)
        segments[digit][SEGMENT_F].start = current_led;
        segments[digit][SEGMENT_F].count = LEDS_PER_SEGMENT_VERTICAL;
        current_led += segments[digit][SEGMENT_F].count;

        // Segment G (middle horizontal)
        segments[digit][SEGMENT_G].start = current_led;
        segments[digit][SEGMENT_G].count = LEDS_PER_SEGMENT_HORIZONTAL;
        current_led += segments[digit][SEGMENT_G].count;
    }

    ESP_LOGI(TAG, "Segment mapping initialized, used %d LEDs", current_led);
}

void PlayClockDisplay::setSegment(uint8_t digit, segment_t segment, bool on) {
    if (digit >= PLAY_CLOCK_DIGITS || !initialized) return;

    uint8_t r, g, b;
    if (on) {
        r = color_on.r;
        g = color_on.g;
        b = color_on.b;
    } else {
        r = color_off.r;
        g = color_off.g;
        b = color_off.b;
    }

    setSegmentColor(digit, segment, r, g, b);
}

void PlayClockDisplay::setSegmentColor(uint8_t digit, segment_t segment, uint8_t r, uint8_t g, uint8_t b) {
    if (digit >= PLAY_CLOCK_DIGITS || segment >= SEGMENTS_PER_DIGIT || !initialized) return;

    uint16_t start = segments[digit][segment].start;
    uint16_t count = segments[digit][segment].count;

    for (uint16_t i = 0; i < count; i++) {
        uint16_t led_index = (start + i) * 3;
        if (led_index + 2 < led_buffer_size) {
            led_buffer[led_index] = r;
            led_buffer[led_index + 1] = g;
            led_buffer[led_index + 2] = b;
        }
    }
}

void PlayClockDisplay::displayDigit(uint8_t digit, uint8_t value) {
    if (digit >= PLAY_CLOCK_DIGITS) return;

    // Clear all segments for this digit first
    for (int seg = 0; seg < SEGMENTS_PER_DIGIT; seg++) {
        setSegment(digit, (segment_t)seg, false);
    }

    // Set segments based on value (0-9 for play clock seconds)
    switch (value) {
        case 0:  // A, B, C, D, E, F
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_C, true);
            setSegment(digit, SEGMENT_D, true);
            setSegment(digit, SEGMENT_E, true);
            setSegment(digit, SEGMENT_F, true);
            break;
        case 1:  // B, C
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_C, true);
            break;
        case 2:  // A, B, G, E, D
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_G, true);
            setSegment(digit, SEGMENT_E, true);
            setSegment(digit, SEGMENT_D, true);
            break;
        case 3:  // A, B, G, C, D
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_G, true);
            setSegment(digit, SEGMENT_C, true);
            setSegment(digit, SEGMENT_D, true);
            break;
        case 4:  // F, G, B, C
            setSegment(digit, SEGMENT_F, true);
            setSegment(digit, SEGMENT_G, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_C, true);
            break;
        case 5:  // A, F, G, C, D
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_F, true);
            setSegment(digit, SEGMENT_G, true);
            setSegment(digit, SEGMENT_C, true);
            setSegment(digit, SEGMENT_D, true);
            break;
        case 6:  // A, F, G, E, C, D
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_F, true);
            setSegment(digit, SEGMENT_G, true);
            setSegment(digit, SEGMENT_E, true);
            setSegment(digit, SEGMENT_C, true);
            setSegment(digit, SEGMENT_D, true);
            break;
        case 7:  // A, B, C
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_C, true);
            break;
        case 8:  // All segments
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_C, true);
            setSegment(digit, SEGMENT_D, true);
            setSegment(digit, SEGMENT_E, true);
            setSegment(digit, SEGMENT_F, true);
            setSegment(digit, SEGMENT_G, true);
            break;
        case 9:  // A, B, C, D, F, G
            setSegment(digit, SEGMENT_A, true);
            setSegment(digit, SEGMENT_B, true);
            setSegment(digit, SEGMENT_C, true);
            setSegment(digit, SEGMENT_D, true);
            setSegment(digit, SEGMENT_F, true);
            setSegment(digit, SEGMENT_G, true);
            break;
    }
}

void PlayClockDisplay::setTime(uint16_t seconds) {
    if (!initialized) return;

    // Play clock shows 2 digits for seconds (00-99)
    uint8_t tens = (seconds % 100) / 10;
    uint8_t ones = seconds % 10;

    displayDigit(0, tens);  // Left digit
    displayDigit(1, ones);  // Right digit
}

void PlayClockDisplay::setLinkStatus(bool connected) {
    link_status = connected;
    if (!connected) {
        current_mode = DISPLAY_MODE_LINK_WARNING;
    }
}

void PlayClockDisplay::setRunMode() {
    current_mode = DISPLAY_MODE_RUN;
}

void PlayClockDisplay::setStopMode() {
    current_mode = DISPLAY_MODE_STOP;
}

void PlayClockDisplay::setResetMode() {
    current_mode = DISPLAY_MODE_RESET;
}

void PlayClockDisplay::showError() {
    current_mode = DISPLAY_MODE_ERROR;
}

void PlayClockDisplay::clearDisplay() {
    if (!initialized) return;
    memset(led_buffer, 0, led_buffer_size);
}

void PlayClockDisplay::showLinkWarning() {
    // Blink middle segments to show link warning
    static bool blink_state = false;
    blink_state = !blink_state;

    setSegment(0, SEGMENT_G, blink_state);
    setSegment(1, SEGMENT_G, blink_state);
}

void PlayClockDisplay::updateLEDStrip() {
    if (!initialized) return;

    // Send LED data via RMT
    rmt_write_items(LED_RMT_CHANNEL, led_buffer, led_buffer_size / 3, false);
}

void PlayClockDisplay::update() {
    if (!initialized) return;

    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Handle different display modes
    switch (current_mode) {
        case DISPLAY_MODE_LINK_WARNING:
            if (current_time - last_update_time > 500) {  // Blink every 500ms
                last_update_time = current_time;
                showLinkWarning();
            }
            break;
        case DISPLAY_MODE_ERROR:
            // Show error pattern (could be implemented)
            break;
        default:
            // Normal operation - just update LEDs
            break;
    }

    updateLEDStrip();
}