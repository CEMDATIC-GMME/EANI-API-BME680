#ifndef REG_ACCESS_H
#define REG_ACCESS_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#define PROTO_I2C 0
#define PROTO_SPI 1

esp_err_t escribir_registro(int protocolo, int page, uint8_t reg, uint8_t value,
                             uint8_t bit_start, uint8_t bit_end);

esp_err_t leer_registro(int protocolo, int page, uint8_t reg,
                         uint8_t bit_start, uint8_t bit_end, uint8_t *value);

esp_err_t reg_read_block(void *dev, uint8_t reg, uint8_t *buf, size_t len); // opcional, no usado por bme680.c

#endif