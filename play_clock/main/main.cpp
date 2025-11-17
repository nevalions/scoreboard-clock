#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "../include/display_driver.h"
#include "../include/radio_comm.h"

static const char* TAG = "PLAY_CLOCK";

#define STATUS_LED_PIN GPIO_NUM_2
#define LINK_TIMEOUT_MS 10000

PlayClockDisplay display;
RadioComm radio;
SystemState system_state;

void setup() {
    ESP_LOGI(TAG, "Starting Play Clock Application");
    
    gpio_reset_pin(STATUS_LED_PIN);
    gpio_set_direction(STATUS_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(STATUS_LED_PIN, 1);
    
    memset(&system_state, 0, sizeof(system_state));
    system_state.last_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    if (!display.begin()) {
        ESP_LOGE(TAG, "Failed to initialize display");
        while (1) {
            gpio_set_level(STATUS_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(STATUS_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    
    if (!radio.begin(NRF24_CE_PIN, NRF24_CSN_PIN)) {
        ESP_LOGE(TAG, "Failed to initialize radio");
        display.showError();
        while (1) {
            gpio_set_level(STATUS_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
            gpio_set_level(STATUS_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
    
    radio.startListening();
    display.setLinkStatus(false);
    display.setStopMode();
    
    ESP_LOGI(TAG, "Play Clock initialized successfully");
}

void loop() {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    if (radio.receiveMessage(system_state)) {
        system_state.last_status_time = current_time;
        system_state.link_alive = true;
        display.setLinkStatus(true);
        
        switch (system_state.display_state) {
            case 0:
                display.setStopMode();
                break;
            case 1:
                display.setRunMode();
                break;
            case 2:
                display.setResetMode();
                break;
            default:
                ESP_LOGW(TAG, "Unknown display state: %d", system_state.display_state);
                break;
        }
        
        display.setTime(system_state.seconds);
        ESP_LOGI(TAG, "State update: mode=%d, time=%d, seq=%d", 
                 system_state.display_state, system_state.seconds, system_state.sequence);
    }
    
    if (current_time - system_state.last_status_time > LINK_TIMEOUT_MS) {
        if (system_state.link_alive) {
            ESP_LOGW(TAG, "Link timeout detected");
            system_state.link_alive = false;
            display.setLinkStatus(false);
        }
    }
    
    display.update();
    
    static bool led_state = false;
    if (system_state.link_alive) {
        if (current_time % 2000 < 1000) {
            led_state = true;
        } else {
            led_state = false;
        }
    } else {
        led_state = (current_time % 500) < 250;
    }
    gpio_set_level(STATUS_LED_PIN, led_state ? 1 : 0);
    
    vTaskDelay(pdMS_TO_TICKS(50));
}

extern "C" void app_main(void) {
    setup();
    
    while (1) {
        loop();
    }
}