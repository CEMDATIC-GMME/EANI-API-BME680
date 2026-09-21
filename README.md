# [EANI] API BME680

## Instrucciones de uso
### I2C/SPI
Para usar I2C o SPI, modificar campo `.protocolo` del struct `bme680_config_t cfg` en `main.c`.

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

### Temperatura heater
Variable `heater_res` en `main.c`. Modificar según parámetro para temperatura objetivo; tercer parámetro para temperatura ambiente.
Ejemplo: 

        uint8_t heater_res = bme680_calc_heater_res(&calib, 320, 25);

<ul>
        <li>Temperatura objetivo >> 320 ºC</li>
        <li>Temperatura ambiente >> 25 ºC</li>
</ul>
