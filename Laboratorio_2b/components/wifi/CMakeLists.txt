idf_component_register(
    SRCS "wifi.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS ""
    PRIV_REQUIRES ""
    REQUIRES
        esp_netif
        esp_wifi
        esp_event
        esp_http_server
)
