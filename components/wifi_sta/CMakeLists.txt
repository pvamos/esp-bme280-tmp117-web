idf_component_register(
    SRCS "wifi_sta.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_wifi esp_netif nvs_flash
)
