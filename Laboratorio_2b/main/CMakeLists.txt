idf_component_register(
    SRCS "Laboratorio_2b.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_netif
        esp_wifi
        esp_event
        nvs_flash
        led_strip
        web_utils
        touchpad
        delay
)
