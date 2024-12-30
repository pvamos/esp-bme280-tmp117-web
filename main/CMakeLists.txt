idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES i2c bme280 tmp117 wifi_sta httpd
)
