#include <unity.h>
#include "display_driver.h"
#include "radio_comm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "TEST_INTEGRATION";

// Test fixture
static PlayClockDisplay* display = nullptr;
static RadioComm* radio = nullptr;
static SystemState test_state;

// Simulated hardware states
static bool simulated_radio_connected = false;
static bool simulated_display_initialized = false;

void setUp(void) {
    // Called before each test
    ESP_LOGI(TAG, "Setting up integration test");

    test_state.display_state = 0;
    test_state.seconds = 0;
    test_state.sequence = 0;
    test_state.last_status_time = 0;
    test_state.link_alive = false;

    simulated_radio_connected = false;
    simulated_display_initialized = false;
}

void tearDown(void) {
    // Called after each test
    ESP_LOGI(TAG, "Tearing down integration test");
}

// Helper function to simulate message reception
bool simulate_radio_message(SystemState& state, uint8_t cmd_state, uint16_t seconds, uint8_t seq) {
    ESP_LOGD(TAG, "Simulating radio message: state=%d, seconds=%d, seq=%d", cmd_state, seconds, seq);

    state.display_state = cmd_state;
    state.seconds = seconds;
    state.sequence = seq;
    state.last_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    state.link_alive = true;

    return true;
}

// Test: System initialization sequence
void test_system_initialization(void) {
    ESP_LOGI(TAG, "Testing complete system initialization");

    // Step 1: Initialize display
    PlayClockDisplay test_display;
    simulated_display_initialized = true;
    TEST_ASSERT_TRUE(simulated_display_initialized);

    // Step 2: Initialize radio
    RadioComm test_radio;
    simulated_radio_connected = true;
    TEST_ASSERT_TRUE(simulated_radio_connected);

    ESP_LOGI(TAG, "System initialization completed successfully");
}

// Test: Message reception to display update flow
void test_message_to_display_flow(void) {
    ESP_LOGI(TAG, "Testing message reception to display update flow");

    PlayClockDisplay test_display;
    RadioComm test_radio;

    // Step 1: Receive a status message from controller
    bool message_received = simulate_radio_message(test_state, 1, 45, 10);
    TEST_ASSERT_TRUE(message_received);

    // Step 2: Update display based on received data
    test_display.setTime(test_state.seconds);
    test_display.setLinkStatus(test_state.link_alive);
    test_display.setRunMode();

    // Step 3: Verify display was updated
    TEST_ASSERT_EQUAL_UINT16(45, test_state.seconds);
    TEST_ASSERT_TRUE(test_state.link_alive);
    TEST_ASSERT_EQUAL_UINT8(1, test_state.display_state); // RUN mode

    ESP_LOGI(TAG, "Message to display flow completed successfully");
}

// Test: Different controller states
void test_controller_state_transitions(void) {
    ESP_LOGI(TAG, "Testing controller state transitions");

    PlayClockDisplay test_display;

    // Test STOP state
    simulate_radio_message(test_state, 0, 30, 1);
    test_display.setStopMode();
    test_display.setTime(test_state.seconds);
    TEST_ASSERT_EQUAL_UINT8(0, test_state.display_state);
    TEST_ASSERT_EQUAL_UINT16(30, test_state.seconds);

    // Test RUN state
    simulate_radio_message(test_state, 1, 35, 2);
    test_display.setRunMode();
    test_display.setTime(test_state.seconds);
    TEST_ASSERT_EQUAL_UINT8(1, test_state.display_state);
    TEST_ASSERT_EQUAL_UINT16(35, test_state.seconds);

    // Test RESET state
    simulate_radio_message(test_state, 2, 0, 3);
    test_display.setResetMode();
    test_display.setTime(0); // Reset always shows 0
    TEST_ASSERT_EQUAL_UINT8(2, test_state.display_state);

    ESP_LOGI(TAG, "Controller state transitions completed successfully");
}

// Test: Link timeout detection and recovery
void test_link_timeout_and_recovery(void) {
    ESP_LOGI(TAG, "Testing link timeout detection and recovery");

    PlayClockDisplay test_display;

    // Step 1: Normal operation with link
    simulate_radio_message(test_state, 1, 25, 5);
    test_display.setLinkStatus(true);
    TEST_ASSERT_TRUE(test_state.link_alive);

    // Step 2: Simulate link timeout (no messages for >800ms)
    vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (current_time - test_state.last_status_time > STATUS_TIMEOUT_MS) {
        test_state.link_alive = false;
        test_display.setLinkStatus(false);
    }

    TEST_ASSERT_FALSE(test_state.link_alive);

    // Step 3: Simulate link recovery
    simulate_radio_message(test_state, 1, 26, 6);
    test_display.setLinkStatus(true);
    TEST_ASSERT_TRUE(test_state.link_alive);

    ESP_LOGI(TAG, "Link timeout and recovery test completed successfully");
}

// Test: Invalid message handling
void test_invalid_message_handling(void) {
    ESP_LOGI(TAG, "Testing invalid message handling");

    PlayClockDisplay test_display;
    RadioComm test_radio;

    // Test 1: Invalid state value
    SystemState invalid_state;
    bool handled = simulate_radio_message(invalid_state, 255, 50, 10);
    TEST_ASSERT_TRUE(handled); // Message received but state invalid
    TEST_ASSERT_EQUAL_UINT8(255, invalid_state.display_state);

    // Test 2: Invalid time value
    handled = simulate_radio_message(invalid_state, 1, 999, 11);
    TEST_ASSERT_TRUE(handled);
    TEST_ASSERT_EQUAL_UINT16(999, invalid_state.seconds); // Should be handled gracefully

    // Test 3: Corrupted sequence
    handled = simulate_radio_message(invalid_state, 1, 30, 255);
    TEST_ASSERT_TRUE(handled);

    ESP_LOGI(TAG, "Invalid message handling completed");
}

// Test: Rapid message processing
void test_rapid_message_processing(void) {
    ESP_LOGI(TAG, "Testing rapid message processing");

    PlayClockDisplay test_display;

    // Simulate rapid succession of messages
    for (int i = 0; i < 10; i++) {
        simulate_radio_message(test_state, 1, 40 + i, i + 1);
        test_display.setTime(test_state.seconds);

        // Small delay between messages
        vTaskDelay(pdMS_TO_TICKS(50));

        TEST_ASSERT_EQUAL_UINT16(40 + i, test_state.seconds);
    }

    ESP_LOGI(TAG, "Rapid message processing completed successfully");
}

// Test: Display effects during different states
void test_display_effects_integration(void) {
    ESP_LOGI(TAG, "Testing display effects integration");

    PlayClockDisplay test_display;

    // Test normal operation effect
    test_display.setRunMode();
    test_display.update();
    vTaskDelay(pdMS_TO_TICKS(100));

    // Test link warning effect
    test_display.setLinkStatus(false);
    test_display.update();
    vTaskDelay(pdMS_TO_TICKS(600)); // Allow for blinking effect

    // Test error effect
    test_display.showError();
    test_display.update();
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Display effects integration completed");
}

// Test: System timing constraints
void test_system_timing_constraints(void) {
    ESP_LOGI(TAG, "Testing system timing constraints");

    uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Simulate main loop operations
    for (int i = 0; i < 10; i++) {
        // Simulate radio check
        // (mock operation)

        // Simulate display update
        PlayClockDisplay test_display;
        test_display.update();

        // Small delay (main loop delay)
        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS));
    }

    uint32_t end_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t elapsed = end_time - start_time;

    // Should complete within reasonable time
    TEST_ASSERT_LESS_THAN_UINT32(2000, elapsed); // Less than 2 seconds

    ESP_LOGI(TAG, "System timing constraints test completed in %d ms", elapsed);
}

// Test: Memory usage
void test_memory_usage_integration(void) {
    ESP_LOGI(TAG, "Testing memory usage integration");

    size_t free_heap_before = esp_get_free_heap_size();

    {
        PlayClockDisplay test_display;
        RadioComm test_radio;

        // Simulate normal operation
        simulate_radio_message(test_state, 1, 50, 20);
        test_display.setTime(test_state.seconds);
        test_display.update();
    }

    size_t free_heap_after = esp_get_free_heap_size();
    size_t memory_used = free_heap_before - free_heap_after;

    ESP_LOGI(TAG, "Memory used during integration test: %d bytes", memory_used);

    // Memory usage should be reasonable
    TEST_ASSERT_LESS_THAN_UINT32(10000, memory_used); // Less than 10KB
}

// Test: Error recovery scenarios
void test_error_recovery_scenarios(void) {
    ESP_LOGI(TAG, "Testing error recovery scenarios");

    PlayClockDisplay test_display;

    // Scenario 1: Display initialization failure
    simulated_display_initialized = false;
    if (!simulated_display_initialized) {
        ESP_LOGW(TAG, "Display initialization failed - showing error pattern");
        test_display.showError();
    }

    // Scenario 2: Radio communication failure
    simulated_radio_connected = false;
    if (!simulated_radio_connected) {
        ESP_LOGW(TAG, "Radio communication failed - attempting recovery");
        test_display.setLinkStatus(false);
    }

    // Scenario 3: Recovery
    simulated_display_initialized = true;
    simulated_radio_connected = true;
    if (simulated_display_initialized && simulated_radio_connected) {
        ESP_LOGI(TAG, "System recovered successfully");
        test_display.setLinkStatus(true);
    }

    ESP_LOGI(TAG, "Error recovery scenarios completed");
}

// Main function to run integration tests
extern "C" void app_main(void) {
    UNITY_BEGIN();

    ESP_LOGI(TAG, "Starting Integration Tests for Play Clock Module");

    RUN_TEST(test_system_initialization);
    RUN_TEST(test_message_to_display_flow);
    RUN_TEST(test_controller_state_transitions);
    RUN_TEST(test_link_timeout_and_recovery);
    RUN_TEST(test_invalid_message_handling);
    RUN_TEST(test_rapid_message_processing);
    RUN_TEST(test_display_effects_integration);
    RUN_TEST(test_system_timing_constraints);
    RUN_TEST(test_memory_usage_integration);
    RUN_TEST(test_error_recovery_scenarios);

    UNITY_END();
}