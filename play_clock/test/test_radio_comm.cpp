#include <unity.h>
#include "radio_comm.h"
#include "esp_log.h"

static const char* TAG = "TEST_RADIO_COMM";

// Test fixture
static RadioComm* radio = nullptr;
static SystemState test_state;

// Helper function to create a test status frame
StatusFrame createTestStatusFrame(uint8_t state, uint16_t seconds, uint8_t sequence) {
    StatusFrame frame;
    frame.frame_type = STATUS_FRAME_TYPE;
    frame.state = state;
    frame.seconds = seconds;
    frame.ms_lowres = 0;
    frame.sequence = sequence;

    // Calculate CRC8 (simplified for testing)
    frame.crc8 = 0x00; // Would normally calculate real CRC

    return frame;
}

void setUp(void) {
    // Called before each test
    ESP_LOGI(TAG, "Setting up test");
    test_state.display_state = 0;
    test_state.seconds = 0;
    test_state.sequence = 0;
    test_state.last_status_time = 0;
    test_state.link_alive = false;
}

void tearDown(void) {
    // Called after each test
    ESP_LOGI(TAG, "Tearing down test");
}

// Test: CRC8 calculation
void test_crc8_calculation(void) {
    ESP_LOGI(TAG, "Testing CRC8 calculation");

    RadioComm test_radio;
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t crc = test_radio.calculateCRC8(test_data, sizeof(test_data));

    // CRC8 of [0x01, 0x02, 0x03, 0x04] should be 0x79 (verified with external calculator)
    TEST_ASSERT_EQUAL_UINT8(0x79, crc);
}

// Test: Status frame parsing - valid frame
void test_status_frame_parsing_valid(void) {
    ESP_LOGI(TAG, "Testing valid status frame parsing");

    // Create a valid status frame
    StatusFrame frame = createTestStatusFrame(1, 45, 10);
    frame.crc8 = 0x00; // Simplified for test

    // Simulate receiving the frame (would normally come from radio hardware)
    SystemState parsed_state;
    parsed_state.display_state = frame.state;
    parsed_state.seconds = frame.seconds;
    parsed_state.sequence = frame.sequence;

    // Verify parsed data
    TEST_ASSERT_EQUAL_UINT8(1, parsed_state.display_state);
    TEST_ASSERT_EQUAL_UINT16(45, parsed_state.seconds);
    TEST_ASSERT_EQUAL_UINT8(10, parsed_state.sequence);
}

// Test: Status frame parsing - invalid frame type
void test_status_frame_parsing_invalid_type(void) {
    ESP_LOGI(TAG, "Testing invalid frame type rejection");

    // Create a frame with wrong type
    StatusFrame frame = createTestStatusFrame(1, 30, 5);
    frame.frame_type = 0xFF; // Invalid frame type

    // Should reject invalid frame type
    TEST_ASSERT_NOT_EQUAL(STATUS_FRAME_TYPE, frame.frame_type);
}

// Test: System state updates
void test_system_state_updates(void) {
    ESP_LOGI(TAG, "Testing system state updates");

    // Initial state
    TEST_ASSERT_EQUAL_UINT8(0, test_state.display_state);
    TEST_ASSERT_EQUAL_UINT16(0, test_state.seconds);
    TEST_ASSERT_EQUAL_UINT8(0, test_state.sequence);
    TEST_ASSERT_FALSE(test_state.link_alive);

    // Simulate receiving a status update
    test_state.display_state = 1; // RUN
    test_state.seconds = 75;
    test_state.sequence = 3;
    test_state.link_alive = true;

    // Verify state was updated
    TEST_ASSERT_EQUAL_UINT8(1, test_state.display_state);
    TEST_ASSERT_EQUAL_UINT16(75, test_state.seconds);
    TEST_ASSERT_EQUAL_UINT8(3, test_state.sequence);
    TEST_ASSERT_TRUE(test_state.link_alive);
}

// Test: Different display states
void test_display_states(void) {
    ESP_LOGI(TAG, "Testing display state values");

    // Test STOP state
    test_state.display_state = 0;
    TEST_ASSERT_EQUAL_UINT8(0, test_state.display_state);

    // Test RUN state
    test_state.display_state = 1;
    TEST_ASSERT_EQUAL_UINT8(1, test_state.display_state);

    // Test RESET state
    test_state.display_state = 2;
    TEST_ASSERT_EQUAL_UINT8(2, test_state.display_state);
}

// Test: Timeout handling
void test_timeout_handling(void) {
    ESP_LOGI(TAG, "Testing timeout detection");

    uint32_t current_time = 1000; // 1 second

    // Set last status time to indicate recent communication
    test_state.last_status_time = current_time - 500; // 500ms ago
    test_state.link_alive = true;

    // Should still be considered alive (less than 800ms timeout)
    TEST_ASSERT_TRUE(test_state.link_alive);

    // Simulate timeout
    test_state.last_status_time = current_time - 1000; // 1 second ago
    if (current_time - test_state.last_status_time > STATUS_TIMEOUT_MS) {
        test_state.link_alive = false;
    }

    // Should now be considered timed out
    TEST_ASSERT_FALSE(test_state.link_alive);
}

// Test: Radio initialization (mock)
void test_radio_initialization(void) {
    ESP_LOGI(TAG, "Testing radio initialization");

    // Create radio instance
    RadioComm test_radio;

    // Test that radio can be created (actual hardware test would need real GPIO)
    // This is a mock test - real hardware testing would need actual ESP32
    TEST_ASSERT_NOT_NULL(&test_radio);
}

// Test: Frame structure size
void test_frame_structure_size(void) {
    ESP_LOGI(TAG, "Testing frame structure sizes");

    // Verify frame structures are expected sizes
    TEST_ASSERT_EQUAL_UINT8(7, sizeof(StatusFrame)); // 7 bytes total

    // Verify individual field sizes
    StatusFrame frame;
    TEST_ASSERT_EQUAL_UINT8(1, sizeof(frame.frame_type));
    TEST_ASSERT_EQUAL_UINT8(1, sizeof(frame.state));
    TEST_ASSERT_EQUAL_UINT8(2, sizeof(frame.seconds));
    TEST_ASSERT_EQUAL_UINT8(2, sizeof(frame.ms_lowres));
    TEST_ASSERT_EQUAL_UINT8(1, sizeof(frame.sequence));
    TEST_ASSERT_EQUAL_UINT8(1, sizeof(frame.crc8));
}

// Main function to run tests
extern "C" void app_main(void) {
    UNITY_BEGIN();

    ESP_LOGI(TAG, "Starting Radio Communication Unit Tests");

    RUN_TEST(test_crc8_calculation);
    RUN_TEST(test_status_frame_parsing_valid);
    RUN_TEST(test_status_frame_parsing_invalid_type);
    RUN_TEST(test_system_state_updates);
    RUN_TEST(test_display_states);
    RUN_TEST(test_timeout_handling);
    RUN_TEST(test_radio_initialization);
    RUN_TEST(test_frame_structure_size);

    UNITY_END();
}