idf_component_register(
    SRCS "Laboratorio_2b.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        led_strip
        web_utils
        touchpad
        delay
        wifi
)
