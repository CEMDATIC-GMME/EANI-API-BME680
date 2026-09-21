#include "reg_access.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include <string.h>

#define PAGE_REG      0xFF   // MODIFICABLE: registro de página, si tu sensor lo usa
#define I2C_SDA_PIN   GPIO_NUM_6   // MODIFICABLE
#define I2C_SCL_PIN   GPIO_NUM_7   // MODIFICABLE
#define I2C_ADDR      0x76         // MODIFICABLE: 0x76 o 0x77 según pin SDO del BME680
#define SPI_MOSI_PIN  GPIO_NUM_19  // MODIFICABLE
#define SPI_MISO_PIN  GPIO_NUM_20  // MODIFICABLE
#define SPI_SCLK_PIN  GPIO_NUM_21  // MODIFICABLE
#define SPI_CS_PIN    GPIO_NUM_18  // MODIFICABLE
#define SPI_MODE      0            // MODIFICABLE
#define SPI_FREQ_HZ   (10*1000*1000) // MODIFICABLE

static i2c_master_dev_handle_t i2c_handle = NULL;
static spi_device_handle_t     spi_handle = NULL;

static void ensure_init(int protocolo) {
    if (protocolo == PROTO_I2C && i2c_handle == NULL) {
        i2c_master_bus_handle_t bus;
        i2c_master_bus_config_t bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = I2C_SDA_PIN,
            .scl_io_num = I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
        };
        i2c_new_master_bus(&bus_cfg, &bus);

        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_ADDR,
            .scl_speed_hz = 400000,
        };
        i2c_master_bus_add_device(bus, &dev_cfg, &i2c_handle);
    }

    if (protocolo == PROTO_SPI && spi_handle == NULL) {
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SPI_MOSI_PIN,
            .miso_io_num = SPI_MISO_PIN,
            .sclk_io_num = SPI_SCLK_PIN,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4096,
        };
        spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

        spi_device_interface_config_t dev_cfg = {
            .clock_speed_hz = SPI_FREQ_HZ,
            .mode = SPI_MODE,
            .spics_io_num = SPI_CS_PIN,
            .queue_size = 1,
        };
        spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi_handle);
    }
}

static esp_err_t raw_write(int protocolo, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = { reg, value };
    if (protocolo == PROTO_I2C)
        return i2c_master_transmit(i2c_handle, buf, sizeof(buf), -1);

    spi_transaction_t t = { .length = 16, .tx_buffer = buf, .rx_buffer = NULL };
    return spi_device_transmit(spi_handle, &t);
}

static esp_err_t raw_read(int protocolo, uint8_t reg, uint8_t *value) {
    if (protocolo == PROTO_I2C)
        return i2c_master_transmit_receive(i2c_handle, &reg, 1, value, 1, -1);

    uint8_t tx[2] = { reg | 0x80, 0x00 }; // MODIFICABLE
    uint8_t rx[2] = {0};
    spi_transaction_t t = { .length = 16, .tx_buffer = tx, .rx_buffer = rx };
    esp_err_t err = spi_device_transmit(spi_handle, &t);
    *value = rx[1];
    return err;
}

esp_err_t escribir_registro(int protocolo, int page, uint8_t reg, uint8_t value,
                             uint8_t bit_start, uint8_t bit_end) {
    ensure_init(protocolo);

    if (page >= 0) {
        esp_err_t err = raw_write(protocolo, PAGE_REG, (uint8_t)page);
        if (err != ESP_OK) return err;
    }

    uint8_t current;
    esp_err_t err = raw_read(protocolo, reg, &current);
    if (err != ESP_OK) return err;

    uint8_t width = bit_end - bit_start + 1;
    uint8_t mask  = ((1 << width) - 1) << bit_start;
    uint8_t new_val = (current & ~mask) | ((value << bit_start) & mask);

    return raw_write(protocolo, reg, new_val);
}

esp_err_t leer_registro(int protocolo, int page, uint8_t reg,
                         uint8_t bit_start, uint8_t bit_end, uint8_t *value) {
    ensure_init(protocolo);

    if (page >= 0) {
        esp_err_t err = raw_write(protocolo, PAGE_REG, (uint8_t)page);
        if (err != ESP_OK) return err;
    }

    uint8_t raw;
    esp_err_t err = raw_read(protocolo, reg, &raw);
    if (err != ESP_OK) return err;

    uint8_t width = bit_end - bit_start + 1;
    uint8_t mask  = ((1 << width) - 1) << bit_start;
    *value = (raw & mask) >> bit_start;
    return ESP_OK;
}

esp_err_t reg_read_block(void *dev, uint8_t reg, uint8_t *buf, size_t len) {
    (void)dev; (void)reg; (void)buf; (void)len;
    return ESP_ERR_NOT_SUPPORTED; // no usado, bme680.c lee todo con leer_registro
}