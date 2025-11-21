#include "../include/radio_comm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "RADIO_COMM";



bool radio_begin(RadioComm *radio, gpio_num_t ce, gpio_num_t csn) {
  ESP_LOGI(TAG, "Initializing radio");

  // Initialize structure
  memset(radio, 0, sizeof(RadioComm));

  radio->ce_pin = ce;
  radio->csn_pin = csn;

  // Initialize addresses
  uint8_t default_tx[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  uint8_t default_rx[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  memcpy(radio->tx_address, default_tx, 5);
  memcpy(radio->rx_address, default_rx, 5);

  // Initialization - just log
  ESP_LOGI(TAG, "Radio CE pin: %d, CSN pin: %d", ce, csn);

  radio->initialized = true;
  return true;
}

bool radio_receive_message(RadioComm *radio, SystemState *state) {
  if (!radio->initialized) {
    ESP_LOGE(TAG, "Radio not initialized");
    return false;
  }

  // Implementation - simulate receiving a message occasionally
  static uint32_t last_mock_time = 0;
  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  if (current_time - last_mock_time > 5000) { // Every 5 seconds
    // Simulate receiving a status message
    state->display_state = 1;                    // RUN
    state->seconds = (current_time / 1000) % 60; // Mock time
    state->sequence = (state->sequence + 1) % 256;
    state->last_status_time = current_time;
    state->link_alive = true;

    last_mock_time = current_time;

    ESP_LOGI(TAG, "Mock message received: state=%d, seconds=%d, seq=%d",
             state->display_state, state->seconds, state->sequence);
    return true;
  }

  return false;
}

void radio_start_listening(RadioComm *radio) {
  if (!radio->initialized)
    return;

  ESP_LOGI(TAG, "Starting radio listening (mock)");
}

void radio_stop_listening(RadioComm *radio) {
  if (!radio->initialized)
    return;

  ESP_LOGI(TAG, "Stopping radio listening (mock)");
}

bool radio_is_data_available(RadioComm *radio) {
  // Mock implementation - return false most of the time
  return false;
}

void radio_flush_rx(RadioComm *radio) {
  ESP_LOGD(TAG, "Flushing RX buffer (mock)");
}


