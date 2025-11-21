#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

// System state structure
typedef struct {
    uint8_t display_state;  // 0=STOP, 1=RUN, 2=RESET
    uint16_t seconds;
    uint8_t sequence;
    uint32_t last_status_time;
    bool link_alive;
} SystemState;

// Radio module configuration (nRF24L01+)
#define NRF24_CE_PIN      GPIO_NUM_5
#define NRF24_CSN_PIN     GPIO_NUM_4

// Radio communication structure
typedef struct {
    bool initialized;
    gpio_num_t ce_pin;
    gpio_num_t csn_pin;

    // Radio configuration
    uint8_t tx_address[5];
    uint8_t rx_address[5];
} RadioComm;

// Function declarations
bool radio_begin(RadioComm* radio, gpio_num_t ce, gpio_num_t csn);
bool radio_receive_message(RadioComm* radio, SystemState* state);
void radio_start_listening(RadioComm* radio);
void radio_stop_listening(RadioComm* radio);
bool radio_is_data_available(RadioComm* radio);
void radio_flush_rx(RadioComm* radio);