idf_component_register(
    SRCS "web_utils.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "esp_http_server"
    PRIV_REQUIRES "esp_netif"
    PRIV_REQUIRES "json"
    PRIV_REQUIRES "espressif__cjson"
)
