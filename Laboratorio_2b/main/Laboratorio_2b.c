#include "delay.h"
#include "esp_err.h"
#include "led_strip.h"
#include "nvs_flash.h"
#include "rgb_led.h"
#include "touchpad.h"
#include "web_utils.h"
#include "wifi.h"
#include <stdio.h>
#include <string.h>

#define AP_SSID "ESP32_AP"
#define AP_PASSWD "embedded" // dejar vacio para red abierta

#define SSID ""
#define PASSWD ""

#define MAX_RETRY 5

void app_main(void) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    led_strip_t *led = NULL;
    ESP_ERROR_CHECK(led_rgb_init(&led));

    touchpad_init();

    wifi_start_ap(AP_SSID, AP_PASSWD);
    // wifi_start_sta(SSID, PASSWD);
    start_webserver(led);

    led_set_power(false);

    bool photo_was_pressed = false;
    bool network_was_pressed = false;
    bool record_was_pressed = false;
    bool play_was_pressed = false;
    bool vol_up_was_pressed = false;
    bool vol_down_was_pressed = false;

    while (1) {
        bool photo_pressed = touchpad_is_pressed(TOUCHPAD_BTN_PHOTO);
        bool network_pressed = touchpad_is_pressed(TOUCHPAD_BTN_NETWORK);
        bool record_pressed = touchpad_is_pressed(TOUCHPAD_BTN_RECORD);
        bool play_pressed = touchpad_is_pressed(TOUCHPAD_BTN_PLAY);
        bool vol_up_pressed = touchpad_is_pressed(TOUCHPAD_BTN_VOLUP);
        bool vol_down_pressed = touchpad_is_pressed(TOUCHPAD_BTN_VOLDOWN);

        if (photo_pressed && !photo_was_pressed) {
            // PHOTO: naranja
            led_update_state(255, 165, 0);
        }

        if (network_pressed && !network_was_pressed) {
            // NETWORK: magenta
            led_update_state(255, 0, 255);
        }

        if (record_pressed && !record_was_pressed) {
            // RECORD: apagada - queda apagado y no deberia cambiar de color desde la web, si guardar el estado uuu
            led_set_power(false);
        }

        if (play_pressed && !play_was_pressed) {
            // PLAY: enciende
            led_set_power(true);
        }

        if (vol_up_pressed && !vol_up_was_pressed && led_get_power()) {
            // VOL_UP: aumenta brillo
            led_brightness_up(led, 10);
        }

        if (vol_down_pressed && !vol_down_was_pressed && led_get_power()) {
            // VOL_DOWN: disminuye brillo
            led_brightness_down(led, 10);
        }

        photo_was_pressed = photo_pressed;
        network_was_pressed = network_pressed;
        record_was_pressed = record_pressed;
        play_was_pressed = play_pressed;
        vol_up_was_pressed = vol_up_pressed;
        vol_down_was_pressed = vol_down_pressed;

        delay_ms(50);
    }
}
