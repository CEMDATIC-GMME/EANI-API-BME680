# [EANI] API BME680
API para usar el BME680 con el ESP32C6. Desarrollada usando VSCode en IDF versión 6.0.2, siguiendo datasheet: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme680-ds001.pdf

Para manipular la librería solo es necesario modificar el archivo `main.c`. En caso de querer hacer otras configuraciones:

<ul>
        <li>bme680.c >> Contiene definiciones de direcciones de memoria del BME680.</li>
        <li>bme680.h >> Contiene variables para seleccionar modos de oversampling y coeficiente filtro IIR.</li>
        <li>reg_access.h >> Establece definición de selección entre I2C/SPI (ver variable `PROTO_I2C/PROTO_SPI`)</li>
        <li>reg_access.c >> Establece las direcciones y pines usados por el ESP32 para I2C/SPI. También la dirección del BME680. </li>
</ul>

## Instrucciones de uso
### I2C/SPI
#### Configuración hardware I2C/SPI
Variables de configuración en `reg_access.c`. Modificar según necesidades.

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

#### Selección I2C/SPI
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
Variable `heater_res` en `main.c`. Modificar segundo parámetro para temperatura objetivo; tercer parámetro para temperatura ambiente.
Ejemplo: 

        uint8_t heater_res = bme680_calc_heater_res(&calib, 320, 25);

<ul>
        <li>Temperatura objetivo >> 320 ºC</li>
        <li>Temperatura ambiente >> 25 ºC</li>
</ul>

### Configuración Filtro IIR y oversampling
En bme680.h se encuentran definidos los structs `bme680_oversampling_t` y `bme680_filter_t`. 

        typedef enum {
            BME680_OS_SKIP = 0, BME680_OS_1X = 1, BME680_OS_2X = 2,
            BME680_OS_4X = 3, BME680_OS_8X = 4, BME680_OS_16X = 5,
        } bme680_oversampling_t;
        
        typedef enum {
            BME680_FILTER_OFF = 0, BME680_FILTER_1X = 1, BME680_FILTER_3X = 2,
            BME680_FILTER_7X = 3, BME680_FILTER_15X = 4, BME680_FILTER_31X = 5,
            BME680_FILTER_63X = 6, BME680_FILTER_127X = 7,
        } bme680_filter_t;

Al igual que I2C/SPI, para modificar estos valores se ha de modificar el struct `bme680_config_t cfg` en `main.c`.
