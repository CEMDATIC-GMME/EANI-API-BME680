#include "bme680.h"
#include "reg_access.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define REG_CHIP_ID     0xD0
#define REG_SOFT_RESET  0xE0
#define REG_STATUS      0x1D
#define REG_CTRL_HUM    0x72
#define REG_CTRL_MEAS   0x74
#define REG_CONFIG      0x75
#define REG_CTRL_GAS_1  0x71
#define REG_RES_HEAT_0  0x5A
#define REG_GAS_WAIT_0  0x64

#define CHIP_ID_VAL     0x61

esp_err_t bme680_check_id(int protocolo, bool *ok) {
    uint8_t id;
    esp_err_t err = leer_registro(protocolo, -1, REG_CHIP_ID, 0, 7, &id);
    if (err != ESP_OK) return err;
    *ok = (id == CHIP_ID_VAL);
    return ESP_OK;
}

static esp_err_t leer_calib_16bits(int protocolo, uint8_t reg_lsb, uint8_t reg_msb, int16_t *out) {
    uint8_t lsb, msb;
    esp_err_t err = leer_registro(protocolo, -1, reg_lsb, 0, 7, &lsb);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, reg_msb, 0, 7, &msb);
    if (err != ESP_OK) return err;
    *out = (int16_t)(msb << 8 | lsb);
    return ESP_OK;
}

esp_err_t bme680_init(int protocolo, bme680_calib_t *calib) {
    esp_err_t err;

    err = escribir_registro(protocolo, -1, REG_SOFT_RESET, 0xB6, 0, 7);
    if (err != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(10));

    bool ok;
    err = bme680_check_id(protocolo, &ok);
    if (err != ESP_OK) return err;
    if (!ok) return ESP_ERR_NOT_FOUND;

    err = leer_calib_16bits(protocolo, 0xE9, 0xEA, (int16_t*)&calib->par_t1);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0x8A, 0x8B, &calib->par_t2);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0x8C, 0, 7, (uint8_t*)&calib->par_t3);
    if (err != ESP_OK) return err;

    err = leer_calib_16bits(protocolo, 0x8E, 0x8F, (int16_t*)&calib->par_p1);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0x90, 0x91, &calib->par_p2);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0x92, 0, 7, (uint8_t*)&calib->par_p3);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0x94, 0x95, &calib->par_p4);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0x96, 0x97, &calib->par_p5);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0x99, 0, 7, (uint8_t*)&calib->par_p6);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0x98, 0, 7, (uint8_t*)&calib->par_p7);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0x9C, 0x9D, &calib->par_p8);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0x9E, 0x9F, &calib->par_p9);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xA0, 0, 7, &calib->par_p10);
    if (err != ESP_OK) return err;

    uint8_t h_e1, h_e2, h_e3;
    err = leer_registro(protocolo, -1, 0xE1, 0, 7, &h_e1);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xE2, 0, 7, &h_e2);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xE3, 0, 7, &h_e3);
    if (err != ESP_OK) return err;
    calib->par_h2 = (uint16_t)((h_e1 << 4) | (h_e2 >> 4));
    calib->par_h1 = (uint16_t)((h_e3 << 4) | (h_e2 & 0x0F));

    err = leer_registro(protocolo, -1, 0xE4, 0, 7, (uint8_t*)&calib->par_h3);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xE5, 0, 7, (uint8_t*)&calib->par_h4);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xE6, 0, 7, (uint8_t*)&calib->par_h5);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xE7, 0, 7, &calib->par_h6);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xE8, 0, 7, (uint8_t*)&calib->par_h7);
    if (err != ESP_OK) return err;

    err = leer_registro(protocolo, -1, 0xED, 0, 7, (uint8_t*)&calib->par_gh1);
    if (err != ESP_OK) return err;
    err = leer_calib_16bits(protocolo, 0xEB, 0xEC, &calib->par_gh2);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0xEE, 0, 7, (uint8_t*)&calib->par_gh3);
    if (err != ESP_OK) return err;

    err = leer_registro(protocolo, -1, 0x02, 4, 5, &calib->res_heat_range);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0x00, 0, 7, (uint8_t*)&calib->res_heat_val);
    if (err != ESP_OK) return err;
    err = leer_registro(protocolo, -1, 0x04, 4, 7, (uint8_t*)&calib->range_sw_err);
    return err;
}

esp_err_t bme680_configurar(const bme680_config_t *cfg) {
    esp_err_t err;

    err = escribir_registro(cfg->protocolo, -1, REG_CTRL_HUM, cfg->os_hum, 0, 2);
    if (err != ESP_OK) return err;
    err = escribir_registro(cfg->protocolo, -1, REG_CONFIG, cfg->filtro, 2, 4);
    if (err != ESP_OK) return err;
    err = escribir_registro(cfg->protocolo, -1, REG_CTRL_MEAS, cfg->os_temp, 5, 7);
    if (err != ESP_OK) return err;
    err = escribir_registro(cfg->protocolo, -1, REG_CTRL_MEAS, cfg->os_pres, 2, 4);
    if (err != ESP_OK) return err;
    err = escribir_registro(cfg->protocolo, -1, REG_RES_HEAT_0, cfg->heater_res, 0, 7);
    if (err != ESP_OK) return err;
    err = escribir_registro(cfg->protocolo, -1, REG_GAS_WAIT_0, cfg->heater_wait_ms, 0, 7);
    if (err != ESP_OK) return err;
    err = escribir_registro(cfg->protocolo, -1, REG_CTRL_GAS_1, cfg->activar_gas ? 1 : 0, 4, 4);
    return err;
}

esp_err_t bme680_trigger_forced(int protocolo) {
    return escribir_registro(protocolo, -1, REG_CTRL_MEAS, 0b01, 0, 1);
}

uint8_t bme680_calc_heater_res(bme680_calib_t *cb, uint16_t target_temp_c, int16_t ambient_temp_c) {
    if (target_temp_c > 400) target_temp_c = 400;

    float var1 = ((float)cb->par_gh1 / 16.0f) + 49.0f;
    float var2 = (((float)cb->par_gh2 / 32768.0f) * 0.0005f) + 0.00235f;
    float var3 = (float)cb->par_gh3 / 1024.0f;
    float var4 = var1 * (1.0f + (var2 * (float)target_temp_c));
    float var5 = var4 + (var3 * (float)ambient_temp_c);

    float res_heat = 3.4f * ((var5 * (4.0f / (4.0f + (float)cb->res_heat_range)) *
                     (1.0f / (1.0f + ((float)cb->res_heat_val * 0.002f)))) - 25.0f);

    return (uint8_t)res_heat;
}

static float calc_temperature(bme680_calib_t *cb, int32_t raw) {
    float var1 = ((float)raw / 16384.0f - (float)cb->par_t1 / 1024.0f) * (float)cb->par_t2;
    float var2 = (((float)raw / 131072.0f - (float)cb->par_t1 / 8192.0f) *
                  ((float)raw / 131072.0f - (float)cb->par_t1 / 8192.0f)) * (float)cb->par_t3 * 16.0f;
    cb->t_fine = (int32_t)(var1 + var2);
    return (var1 + var2) / 5120.0f;
}

static float calc_pressure(bme680_calib_t *cb, int32_t raw) {
    float var1 = ((float)cb->t_fine / 2.0f) - 64000.0f;
    float var2 = var1 * var1 * ((float)cb->par_p6 / 131072.0f);
    var2 += var1 * (float)cb->par_p5 * 2.0f;
    var2 = (var2 / 4.0f) + ((float)cb->par_p4 * 65536.0f);
    var1 = (((float)cb->par_p3 * var1 * var1 / 16384.0f) + ((float)cb->par_p2 * var1)) / 524288.0f;
    var1 = (1.0f + var1 / 32768.0f) * (float)cb->par_p1;
    float press = 1048576.0f - (float)raw;
    press = (press - (var2 / 4096.0f)) * 6250.0f / var1;
    var1 = (float)cb->par_p9 * press * press / 2147483648.0f;
    var2 = press * ((float)cb->par_p8 / 32768.0f);
    float var3 = (press / 256.0f) * (press / 256.0f) * (press / 256.0f) * ((float)cb->par_p10 / 131072.0f);
    press += (var1 + var2 + var3 + ((float)cb->par_p7 * 128.0f)) / 16.0f;
    return press / 100.0f;
}

static float calc_humidity(bme680_calib_t *cb, uint16_t raw, float temp_c) {
    float var1 = (float)raw - (((float)cb->par_h1 * 16.0f) + (((float)cb->par_h3 / 2.0f) * temp_c));
    float var2 = var1 * (((float)cb->par_h2 / 262144.0f) *
                 (1.0f + (((float)cb->par_h4 / 16384.0f) * temp_c) +
                  (((float)cb->par_h5 / 1048576.0f) * temp_c * temp_c)));
    float var3 = (float)cb->par_h6 / 16384.0f;
    float var4 = (float)cb->par_h7 / 2097152.0f;
    float hum = var2 + ((var3 + (var4 * temp_c)) * var2 * var2);
    if (hum > 100.0f) hum = 100.0f;
    if (hum < 0.0f) hum = 0.0f;
    return hum;
}

static float calc_gas_resistance(bme680_calib_t *cb, uint16_t raw_gas, uint8_t gas_range) {
    static const float lookup1[16] = {1,1,1,1,1,0.99f,1,0.992f,1,1,0.998f,0.995f,1,0.99f,1,1};
    static const float lookup2[16] = {8000000,4000000,2000000,1000000,499500.4f,248262.1f,125000,63004.03f,
                                       31281.28f,15625,7812.5f,3906.25f,1953.125f,976.5625f,488.28125f,244.140625f};
    float var1 = (1340.0f + 5.0f * (float)cb->range_sw_err) * lookup1[gas_range];
    return var1 * lookup2[gas_range] / ((float)raw_gas - 512.0f + var1);
}

esp_err_t bme680_leer_medidas(int protocolo, bme680_calib_t *calib, bme680_datos_t *out) {
    esp_err_t err = bme680_trigger_forced(protocolo);
    if (err != ESP_OK) return err;

    uint8_t new_data;
    int intentos = 0;
    do {
        vTaskDelay(pdMS_TO_TICKS(10));
        err = leer_registro(protocolo, -1, REG_STATUS, 7, 7, &new_data);
        if (err != ESP_OK) return err;
        if (++intentos > 20) return ESP_ERR_TIMEOUT;
    } while (new_data == 0);

    uint8_t buf[15];
    for (int i = 0; i < 15; i++) {
        err = leer_registro(protocolo, -1, REG_STATUS + i, 0, 7, &buf[i]);
        if (err != ESP_OK) return err;
    }

    int32_t raw_press = (buf[2] << 12) | (buf[3] << 4) | (buf[4] >> 4);
    int32_t raw_temp   = (buf[5] << 12) | (buf[6] << 4) | (buf[7] >> 4);
    uint16_t raw_hum    = (buf[8] << 8) | buf[9];
    uint16_t raw_gas    = (buf[13] << 2) | (buf[14] >> 6);
    uint8_t  gas_range  = buf[14] & 0x0F;

    out->temperature_c      = calc_temperature(calib, raw_temp);
    out->pressure_hpa       = calc_pressure(calib, raw_press);
    out->humidity_pct       = calc_humidity(calib, raw_hum, out->temperature_c);
    out->gas_resistance_ohm = calc_gas_resistance(calib, raw_gas, gas_range);
    return ESP_OK;
}