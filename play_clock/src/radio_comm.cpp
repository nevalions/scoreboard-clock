#include "radio_comm.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "RADIO_COMM";

// nRF24L01+ register definitions
#define CONFIG_REG      0x00
#define EN_AA_REG       0x01
#define EN_RXADDR_REG   0x02
#define SETUP_AW_REG    0x03
#define SETUP_RETR_REG  0x04
#define RF_CH_REG       0x05
#define RF_SETUP_REG    0x06
#define STATUS_REG      0x07
#define RX_ADDR_P0_REG  0x0A
#define RX_ADDR_P1_REG  0x0B
#define TX_ADDR_REG     0x10
#define RX_PW_P0_REG    0x11
#define RX_PW_P1_REG    0x12
#define FIFO_STATUS_REG 0x17

// nRF24L01+ commands
#define R_REGISTER      0x00
#define W_REGISTER      0x20
#define RX_PAYLOAD      0x61
#define TX_PAYLOAD      0xA0
#define FLUSH_TX        0xE1
#define FLUSH_RX        0xE2
#define ACTIVATE        0x50
#define R_RX_PL_WID     0x60

RadioComm::RadioComm() :
    initialized(false),
    spi(nullptr),
    ce_pin(GPIO_NUM_NC),
    csn_pin(GPIO_NUM_NC),
    current_channel(NRF24_CHANNEL)
{
    // Initialize addresses
    tx_address[0] = 0xE7; tx_address[1] = 0xE7; tx_address[2] = 0xE7; tx_address[3] = 0xE7; tx_address[4] = 0xE7;
    rx_address[0] = 0xC2; rx_address[1] = 0xC2; rx_address[2] = 0xC2; rx_address[3] = 0xC2; rx_address[4] = 0xC2;
}

RadioComm::~RadioComm() {
    if (spi) {
        spi_bus_remove_device(spi);
    }
}

bool RadioComm::begin(gpio_num_t ce, gpio_num_t csn) {
    ESP_LOGI(TAG, "Initializing Radio Communication...");

    ce_pin = ce;
    csn_pin = csn;

    // Initialize GPIO pins
    gpio_reset_pin(ce_pin);
    gpio_set_direction(ce_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(ce_pin, 0);

    gpio_reset_pin(csn_pin);
    gpio_set_direction(csn_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(csn_pin, 1);

    // Initialize SPI
    if (!initSPI()) {
        ESP_LOGE(TAG, "Failed to initialize SPI");
        return false;
    }

    // Initialize radio
    if (!initRadio()) {
        ESP_LOGE(TAG, "Failed to initialize radio");
        return false;
    }

    initialized = true;
    ESP_LOGI(TAG, "Radio Communication initialized successfully");
    return true;
}

bool RadioComm::initSPI() {
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = NRF24_MOSI_PIN,
        .miso_io_num = NRF24_MISO_PIN,
        .sclk_io_num = NRF24_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    esp_err_t ret = spi_bus_initialize(NRF24_SPI_HOST, &bus_cfg, VSPI_HOST);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI bus initialization failed: %s", esp_err_to_name(ret));
        return false;
    }

    spi_device_interface_config_t dev_cfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 1000000,  // 1MHz
        .input_delay_ns = 0,
        .spics_io_num = -1,  // Manual CS control
        .flags = SPI_DEVICE_NO_DUMMY
    };

    ret = spi_bus_add_device(NRF24_SPI_HOST, &dev_cfg, &spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device addition failed: %s", esp_err_to_name(ret));
        return false;
    }

    return true;
}

bool RadioComm::initRadio() {
    csnLow();
    vTaskDelay(pdMS_TO_TICKS(10));

    // Power down the radio
    writeRegister(CONFIG_REG, 0x00);
    vTaskDelay(pdMS_TO_TICKS(5));

    // Enable auto-acknowledgment on pipe 0
    writeRegister(EN_AA_REG, 0x01);

    // Enable RX pipe 0
    writeRegister(EN_RXADDR_REG, 0x01);

    // Set address width to 5 bytes
    writeRegister(SETUP_AW_REG, 0x03);

    // Set retransmission: 15 retries, 1500us delay
    writeRegister(SETUP_RETR_REG, 0x4F);

    // Set channel
    writeRegister(RF_CH_REG, current_channel);

    // Set RF data rate to 250kbps and power level
    writeRegister(RF_SETUP_REG, 0x26);  // 250kbps, 0dBm

    // Set RX address for pipe 0
    writeRegisterMulti(RX_ADDR_P0_REG, rx_address, 5);

    // Set TX address
    writeRegisterMulti(TX_ADDR_REG, tx_address, 5);

    // Set RX payload width for pipe 0
    writeRegister(RX_PW_P0_REG, MAX_PAYLOAD_SIZE);

    // Enable CRC (2 bytes)
    writeRegister(CONFIG_REG, 0x0C);

    // Power up and set to RX mode
    powerUp();
    setRXMode();

    csnHigh();
    vTaskDelay(pdMS_TO_TICKS(5));

    return true;
}

void RadioComm::ceHigh() {
    gpio_set_level(ce_pin, 1);
}

void RadioComm::ceLow() {
    gpio_set_level(ce_pin, 0);
}

void RadioComm::csnLow() {
    gpio_set_level(csn_pin, 0);
}

void RadioComm::csnHigh() {
    gpio_set_level(csn_pin, 1);
}

uint8_t RadioComm::spiTransfer(uint8_t data) {
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &data,
        .rx_buffer = &data
    };

    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(ret));
        return 0;
    }

    return data;
}

void RadioComm::writeRegister(uint8_t reg, uint8_t value) {
    csnLow();
    spiTransfer(W_REGISTER | (reg & 0x1F));
    spiTransfer(value);
    csnHigh();
}

uint8_t RadioComm::readRegister(uint8_t reg) {
    csnLow();
    spiTransfer(R_REGISTER | (reg & 0x1F));
    uint8_t value = spiTransfer(0xFF);
    csnHigh();
    return value;
}

void RadioComm::writeRegisterMulti(uint8_t reg, const uint8_t* data, uint8_t len) {
    csnLow();
    spiTransfer(W_REGISTER | (reg & 0x1F));
    for (uint8_t i = 0; i < len; i++) {
        spiTransfer(data[i]);
    }
    csnHigh();
}

void RadioComm::readRegisterMulti(uint8_t reg, uint8_t* data, uint8_t len) {
    csnLow();
    spiTransfer(R_REGISTER | (reg & 0x1F));
    for (uint8_t i = 0; i < len; i++) {
        data[i] = spiTransfer(0xFF);
    }
    csnHigh();
}

uint8_t RadioComm::calculateCRC8(const uint8_t* data, uint8_t len) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void RadioComm::powerUp() {
    uint8_t config = readRegister(CONFIG_REG);
    config |= 0x02;  // PWR_UP bit
    writeRegister(CONFIG_REG, config);
    vTaskDelay(pdMS_TO_TICKS(5));
}

void RadioComm::setRXMode() {
    ceLow();
    uint8_t config = readRegister(CONFIG_REG);
    config |= 0x01;  // PRIM_RX bit
    writeRegister(CONFIG_REG, config);
    ceHigh();
    vTaskDelay(pdMS_TO_TICKS(1));
}

bool RadioComm::isDataAvailable() {
    uint8_t status = readRegister(STATUS_REG);
    return (status & 0x40) != 0;  // RX_DR bit
}

void RadioComm::flushRX() {
    csnLow();
    spiTransfer(FLUSH_RX);
    csnHigh();
}

bool RadioComm::receiveMessage(SystemState& state) {
    if (!initialized) return false;

    if (!isDataAvailable()) return false;

    // Read payload
    uint8_t payload[MAX_PAYLOAD_SIZE];
    csnLow();
    spiTransfer(RX_PAYLOAD);
    for (int i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        payload[i] = spiTransfer(0xFF);
    }
    csnHigh();

    // Clear RX_DR bit
    uint8_t status = readRegister(STATUS_REG);
    writeRegister(STATUS_REG, status | 0x40);

    // Parse status frame
    if (payload[0] == STATUS_FRAME_TYPE) {
        StatusFrame* frame = (StatusFrame*)payload;

        // Verify CRC
        uint8_t calculated_crc = calculateCRC8(payload, sizeof(StatusFrame) - 1);
        if (calculated_crc != frame->crc8) {
            ESP_LOGW(TAG, "CRC mismatch in received frame");
            return false;
        }

        // Update system state
        state.display_state = frame->state;
        state.seconds = frame->seconds;
        state.sequence = frame->sequence;

        return true;
    }

    return false;
}

void RadioComm::startListening() {
    setRXMode();
}

void RadioComm::stopListening() {
    ceLow();
    uint8_t config = readRegister(CONFIG_REG);
    config &= ~0x01;  // Clear PRIM_RX bit
    writeRegister(CONFIG_REG, config);
}