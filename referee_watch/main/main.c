#include "esp_adc/adc_oneshot.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "espnow_link.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "watch_config.h"
#include <string.h>

static const char *TAG = "WATCH";

#define BUTTON_DEBOUNCE_MS 40
#define SEND_RETRY_MAX 5
#define SEND_RETRY_GAP_MS 30
#define VIBRO_OK_MS 120
#define VIBRO_FAIL_PULSE_MS 80
#define VIBRO_FAIL_PULSES 3
#define MAIN_LOOP_DELAY_MS 20
#define BATTERY_CHECK_INTERVAL_MS 600000 // 10 min

static const uint8_t controller_mac[6] = CONTROLLER_MAC;
static uint8_t tx_sequence = 0;

// Set by the ESP-NOW send callback (WiFi task context)
static volatile int8_t last_send_status = -1; // -1 pending, 0 fail, 1 ok

static void espnow_send_cb(const esp_now_send_info_t *info,
                           esp_now_send_status_t status) {
  (void)info;
  last_send_status = (status == ESP_NOW_SEND_SUCCESS) ? 1 : 0;
}

static void vibro_pulse(uint32_t ms) {
  gpio_set_level(VIBRO_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(ms));
  gpio_set_level(VIBRO_PIN, 0);
}

static void vibro_fail_pattern(void) {
  for (int i = 0; i < VIBRO_FAIL_PULSES; i++) {
    vibro_pulse(VIBRO_FAIL_PULSE_MS);
    vTaskDelay(pdMS_TO_TICKS(VIBRO_FAIL_PULSE_MS));
  }
}

// Send one command with a retry burst; haptic feedback tells the referee
// whether the controller's MAC-layer ACK came back
static void send_command(uint8_t command) {
  EspNowCommand cmd = {
      .magic = ESPNOW_CMD_MAGIC,
      .watch_id = WATCH_ID,
      .command = command,
      .sequence = ++tx_sequence,
  };

  bool delivered = false;
  for (int attempt = 0; attempt < SEND_RETRY_MAX && !delivered; attempt++) {
    last_send_status = -1;
    if (esp_now_send(controller_mac, (const uint8_t *)&cmd, sizeof(cmd)) !=
        ESP_OK) {
      vTaskDelay(pdMS_TO_TICKS(SEND_RETRY_GAP_MS));
      continue;
    }
    // Send callback fires within a few ms
    for (int wait = 0; wait < 20 && last_send_status == -1; wait++) {
      vTaskDelay(pdMS_TO_TICKS(2));
    }
    delivered = (last_send_status == 1);
    if (!delivered) {
      vTaskDelay(pdMS_TO_TICKS(SEND_RETRY_GAP_MS));
    }
  }

  ESP_LOGI(TAG, "cmd=%u seq=%u %s", command, cmd.sequence,
           delivered ? "DELIVERED" : "FAILED");
  if (delivered) {
    vibro_pulse(VIBRO_OK_MS);
  } else {
    vibro_fail_pattern();
  }
}

static void init_wifi_espnow(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(
      esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));

  uint8_t mac[6];
  ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
  ESP_LOGI(TAG, "Watch MAC %02x:%02x:%02x:%02x:%02x:%02x (put this in the "
                "controller's espnow_watches.h)",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  ESP_ERROR_CHECK(esp_now_init());
  ESP_ERROR_CHECK(esp_now_set_pmk((const uint8_t *)ESPNOW_PMK));
  ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

  esp_now_peer_info_t peer = {0};
  memcpy(peer.peer_addr, controller_mac, 6);
  peer.channel = ESPNOW_WIFI_CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = true;
  memcpy(peer.lmk, ESPNOW_LMK, 16);
  ESP_ERROR_CHECK(esp_now_add_peer(&peer));
}

static void init_gpio(void) {
  gpio_config_t btn_conf = {
      .pin_bit_mask =
          (1ULL << BUTTON_START_STOP_PIN) | (1ULL << BUTTON_RESET_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&btn_conf);

  gpio_config_t vib_conf = {
      .pin_bit_mask = (1ULL << VIBRO_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&vib_conf);
  gpio_set_level(VIBRO_PIN, 0);

  // Light-sleep wake on either button (active-low)
  gpio_wakeup_enable(BUTTON_START_STOP_PIN, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable(BUTTON_RESET_PIN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
}

static void check_battery(adc_oneshot_unit_handle_t adc) {
  int raw = 0;
  if (adc_oneshot_read(adc, BATTERY_ADC_CHANNEL, &raw) != ESP_OK) {
    return;
  }
  // 12-bit, 11dB atten: ~3100mV full scale (rough; a warning threshold,
  // not a fuel gauge)
  int mv = (raw * 3100) / 4095;
  ESP_LOGI(TAG, "Battery divider: %d mV", mv);
  if (mv > 0 && mv < BATTERY_WARN_MV) {
    ESP_LOGW(TAG, "Battery low");
    vibro_fail_pattern();
    vTaskDelay(pdMS_TO_TICKS(300));
    vibro_fail_pattern();
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "Referee watch starting (id %d)", WATCH_ID);

  static const uint8_t zero_mac[6] = {0};
  if (memcmp(controller_mac, zero_mac, 6) == 0) {
    ESP_LOGE(TAG, "CONTROLLER_MAC is unset - edit watch_config.h, see the "
                  "controller boot log for its MAC. Halting.");
    // Still bring WiFi up so this unit's own MAC gets logged for pairing
    init_wifi_espnow();
    while (1) {
      vTaskDelay(pdMS_TO_TICKS(10000));
    }
  }

  init_gpio();
  init_wifi_espnow();

  adc_oneshot_unit_handle_t adc = NULL;
  adc_oneshot_unit_init_cfg_t adc_cfg = {.unit_id = ADC_UNIT_1};
  if (adc_oneshot_new_unit(&adc_cfg, &adc) == ESP_OK) {
    adc_oneshot_chan_cfg_t ch_cfg = {.atten = ADC_ATTEN_DB_12,
                                     .bitwidth = ADC_BITWIDTH_12};
    adc_oneshot_config_channel(adc, BATTERY_ADC_CHANNEL, &ch_cfg);
  }

  // Boot-complete haptic
  vibro_pulse(VIBRO_OK_MS);

  bool start_was_pressed = false;
  bool reset_was_pressed = false;
  uint32_t last_battery_check = 0;

  while (1) {
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    bool start_pressed = (gpio_get_level(BUTTON_START_STOP_PIN) == 0);
    bool reset_pressed = (gpio_get_level(BUTTON_RESET_PIN) == 0);

    if (start_pressed && !start_was_pressed) {
      vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS));
      if (gpio_get_level(BUTTON_START_STOP_PIN) == 0) {
        send_command(ESPNOW_CMD_START_STOP);
      }
    }
    if (reset_pressed && !reset_was_pressed) {
      vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS));
      if (gpio_get_level(BUTTON_RESET_PIN) == 0) {
        send_command(ESPNOW_CMD_RESET);
      }
    }
    start_was_pressed = start_pressed;
    reset_was_pressed = reset_pressed;

    if (now - last_battery_check > BATTERY_CHECK_INTERVAL_MS) {
      last_battery_check = now;
      if (adc) {
        check_battery(adc);
      }
    }

    // Idle with both buttons up: light sleep until a button pulls its
    // wake line low (~50ms press-to-send). WiFi stays associated-less
    // (ESP-NOW needs no association) and re-sends work immediately on wake
    if (!start_pressed && !reset_pressed) {
      esp_light_sleep_start();
    }

    vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS));
  }
}
