#include "../components/delay/include/delay.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led_strip.h"
#include "rgb_led.h"
#include "touchpad.h"

#define BLINK_PERIOD_MS 1000

led_strip_t *led;

void app_main(void) {
    touchpad_init();

    if (led_rgb_init(&led) != ESP_OK) {
        ESP_LOGE("ERR", "No se pudo inicializar el driver LED.\n");
    }

    const color_t orange = {255, 165, 0};
    const color_t magenta = {255, 0, 255};

    led_off(led);

    while (1) {
        if (touchpad_is_pressed(TOUCHPAD_BTN_PHOTO)) {
            led_set_color(led, orange);
        } else if (touchpad_is_pressed(TOUCHPAD_BTN_PLAY)) {
            led_on(led);
        } else if (touchpad_is_pressed(TOUCHPAD_BTN_NETWORK)) {
            led_set_color(led, magenta);
        } else if (touchpad_is_pressed(TOUCHPAD_BTN_RECORD)) {
            led_off(led);
        } else if (touchpad_is_pressed(TOUCHPAD_BTN_VOLUP)) {
            led_brightness_up(led, 1);
        } else if (touchpad_is_pressed(TOUCHPAD_BTN_VOLDOWN)) {
            led_brightness_down(led, 1);
        }
    }

    // while (1) {
    //     touchpad_debug_print();
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}
