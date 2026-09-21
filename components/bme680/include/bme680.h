#ifndef BME680_H
#define BME680_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BME680_OS_SKIP = 0, BME680_OS_1X = 1, BME680_OS_2X = 2,
    BME680_OS_4X = 3, BME680_OS_8X = 4, BME680_OS_16X = 5,
} bme680_oversampling_t;

typedef enum {
    BME680_FILTER_OFF = 0, BME680_FILTER_1X = 1, BME680_FILTER_3X = 2,
    BME680_FILTER_7X = 3, BME680_FILTER_15X = 4, BME680_FILTER_31X = 5,
    BME680_FILTER_63X = 6, BME680_FILTER_127X = 7,
} bme680_filter_t;

typedef struct {
    int protocolo;
    bme680_oversampling_t os_temp, os_pres, os_hum;
    bme680_filter_t filtro;
    uint8_t heater_res;
    uint8_t heater_wait_ms;
    bool activar_gas;
} bme680_config_t;

typedef struct {
    uint16_t par_t1; int16_t par_t2; int8_t par_t3;
    uint16_t par_p1; int16_t par_p2; int8_t par_p3; int16_t par_p4;
    int16_t par_p5; int8_t par_p6; int8_t par_p7; int16_t par_p8;
    int16_t par_p9; uint8_t par_p10;
    uint16_t par_h1; uint16_t par_h2; int8_t par_h3; int8_t par_h4;
    int8_t par_h5; uint8_t par_h6; int8_t par_h7;
    int8_t par_gh1; int16_t par_gh2; int8_t par_gh3;
    uint8_t res_heat_range; int8_t res_heat_val; int8_t range_sw_err;
    int32_t t_fine;
} bme680_calib_t;

typedef struct {
    float temperature_c;
    float pressure_hpa;
    float humidity_pct;
    float gas_resistance_ohm;
} bme680_datos_t;

esp_err_t bme680_check_id(int protocolo, bool *ok);
esp_err_t bme680_init(int protocolo, bme680_calib_t *calib);
esp_err_t bme680_configurar(const bme680_config_t *cfg);
esp_err_t bme680_trigger_forced(int protocolo);
esp_err_t bme680_leer_medidas(int protocolo, bme680_calib_t *calib, bme680_datos_t *out);
uint8_t   bme680_calc_heater_res(bme680_calib_t *calib, uint16_t target_temp_c, int16_t ambient_temp_c);

#endif