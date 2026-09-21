idf_component_register(
    SRCS "reg_access.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c esp_driver_spi
)