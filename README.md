# [EANI] API BME680

## Instrucciones de uso
Configuración automática de uso I2C/SPI según variable .protocolo en main.c

        bme680_config_t cfg = {
              .protocolo      = PROTO_I2C, // PROTO_I2C o PROTO_SPI 
              ...
        };
