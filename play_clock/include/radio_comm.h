#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"

// System state structure
typedef struct {
    uint8_t display_state;  // 0=STOP, 1=RUN, 2=RESET
    uint16_t seconds;
    uint8_t sequence;
    uint32_t last_status_time;
    bool link_alive;
} SystemState;

// Radio module configuration (nRF24L01+)
#define NRF24_SPI_HOST    VSPI_HOST
#define NRF24_MOSI_PIN    GPIO_NUM_23
#define NRF24_MISO_PIN    GPIO_NUM_19
#define NRF24_SCK_PIN     GPIO_NUM_18
#define NRF24_CE_PIN      GPIO_NUM_5
#define NRF24_CSN_PIN     GPIO_NUM_4

// Radio configuration
#define NRF24_CHANNEL     100
#define NRF24_DATA_RATE   RF24_250KBPS
#define NRF24_PA_LEVEL    RF24_PA_HIGH
#define NRF24_ADDRESS_WIDTH 5

// Protocol constants
#define STATUS_FRAME_TYPE   0xA1
#define COMMAND_FRAME_TYPE  0xB1
#define MAX_PAYLOAD_SIZE    32

// Mesh node IDs
#define CONTROLLER_NODE_ID  0
#define PLAYCLOCK_NODE_ID   1

// Status broadcast frame structure (from controller)
typedef struct __attribute__((packed)) {
    uint8_t frame_type;    // 0xA1
    uint8_t state;         // 0=STOP, 1=RUN, 2=RESET
    uint16_t seconds;      // Current time in seconds
    uint16_t ms_lowres;    // Low resolution milliseconds
    uint8_t sequence;      // Sequence number
    uint8_t crc8;          // CRC checksum
} StatusFrame;

// Radio communication structure
typedef struct {
    bool initialized;
    spi_device_handle_t spi;
    gpio_num_t ce_pin;
    gpio_num_t csn_pin;

    // Radio configuration
    uint8_t tx_address[5];
    uint8_t rx_address[5];
    uint8_t current_channel;
} RadioComm;

// Function declarations
bool radio_begin(RadioComm* radio, gpio_num_t ce, gpio_num_t csn);
bool radio_receive_message(RadioComm* radio, SystemState* state);
void radio_start_listening(RadioComm* radio);
void radio_stop_listening(RadioComm* radio);
bool radio_is_data_available(RadioComm* radio);
void radio_flush_rx(RadioComm* radio);