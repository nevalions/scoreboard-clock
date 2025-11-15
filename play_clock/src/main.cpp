#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display_driver.h"
#include "radio_comm.h"

static const char* TAG = "PLAY_CLOCK";

// Timing constants
#define STATUS_TIMEOUT_MS 800
#define MAIN_LOOP_DELAY_MS 10

// Pin definitions
#define LED_STATUS_PIN GPIO_NUM_2
#define RADIO_CE_PIN GPIO_NUM_4
#define RADIO_CSN_PIN GPIO_NUM_5

struct SystemState {
    uint8_t display_state;  // 0=STOP, 1=RUN, 2=RESET
    uint16_t seconds;
    uint8_t sequence;
    uint32_t last_status_time;
    bool link_alive;
} system_state;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Play Clock Module Starting...");

    // Initialize system state
    system_state.display_state = 0;
    system_state.seconds = 0;
    system_state.sequence = 0;
    system_state.last_status_time = 0;
    system_state.link_alive = false;

    // Initialize status LED
    gpio_reset_pin(LED_STATUS_PIN);
    gpio_set_direction(LED_STATUS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_STATUS_PIN, 0);

    // Initialize display
    PlayClockDisplay display;
    if (!display.begin()) {
        ESP_LOGE(TAG, "Failed to initialize LED display");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Initialize radio communication
    RadioComm radio;
    if (!radio.begin(RADIO_CE_PIN, RADIO_CSN_PIN)) {
        ESP_LOGE(TAG, "Failed to initialize radio");
        display.showError();  // Show error pattern on display
        while (1) {
            gpio_set_level(LED_STATUS_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(LED_STATUS_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }

    // Set initial display
    display.setTime(0);
    display.setLinkStatus(false);

    ESP_LOGI(TAG, "Play Clock Module Ready");

    while (1) {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // Check for incoming radio messages from controller
        if (radio.receiveMessage(system_state)) {
            system_state.last_status_time = current_time;
            system_state.link_alive = true;
            gpio_set_level(LED_STATUS_PIN, 1);

            // Update display based on received data from controller
            display.setTime(system_state.seconds);
            display.setLinkStatus(true);

            // Handle display state changes based on controller state
            switch (system_state.display_state) {
                case 0:  // STOP
                    display.setStopMode();
                    break;
                case 1:  // RUN
                    display.setRunMode();
                    break;
                case 2:  // RESET
                    display.setTime(0);
                    display.setResetMode();
                    break;
            }

            ESP_LOGI(TAG, "Received from controller: State=%d, Time=%d, Seq=%d",
                     system_state.display_state, system_state.seconds, system_state.sequence);
        }

        // Check for link timeout - no messages from controller
        if (current_time - system_state.last_status_time > STATUS_TIMEOUT_MS) {
            if (system_state.link_alive) {
                system_state.link_alive = false;
                display.setLinkStatus(false);
                gpio_set_level(LED_STATUS_PIN, 0);
                ESP_LOGW(TAG, "Controller link timeout - showing warning");
            }
        }

        // Update display effects
        display.update();

        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS));
    }
}