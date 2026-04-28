#ifndef RGB_LED
#define RGB_LED

#include "esp_err.h"
#include "led_strip.h"
#include <stdint.h>

typedef struct color_s {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

esp_err_t create_led(led_strip_t* led);
esp_err_t led_set_color(led_strip_t* led, color_t colors);

#endif // !RGB_LED
