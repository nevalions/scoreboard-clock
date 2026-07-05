idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "../include" "../../radio-common/include"
    REQUIRES driver esp_wifi esp_netif nvs_flash esp_adc esp_driver_gpio
)
