#include "reg_access.h"
#include "bme680.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BME680";

void app_main(void) {
    bme680_calib_t calib;

    esp_err_t err = bme680_init(PROTO_I2C, &calib);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al inicializar. ¿Está el BME conectado...? %d", err);
        return;
    }
    ESP_LOGI(TAG, "BME680 detectado correctamente");

    uint8_t heater_res = bme680_calc_heater_res(&calib, 320, 25);

    bme680_config_t cfg = {
        .protocolo      = PROTO_I2C, // PROTO_I2C o PROTO_SPI
        .os_temp        = BME680_OS_2X,
        .os_pres        = BME680_OS_4X,
        .os_hum         = BME680_OS_2X,
        .filtro         = BME680_FILTER_3X,
        .heater_res     = heater_res,
        .heater_wait_ms = 0x59,
        .activar_gas    = true,
    };
    err = bme680_configurar(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al configurar: %d", err);
        return;
    }

    while (1) {
        bme680_datos_t datos;
        err = bme680_leer_medidas(PROTO_I2C, &calib, &datos);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "T=%.2f°C  P=%.2fhPa  H=%.2f%%  Gas=%.0fΩ",
                     datos.temperature_c, datos.pressure_hpa,
                     datos.humidity_pct, datos.gas_resistance_ohm);
        } else {
            ESP_LOGW(TAG, "Error leyendo medidas: se han quedado en Ormuz. %d", err);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}