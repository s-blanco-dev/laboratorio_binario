#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include "esp_http_server.h"
#include "led_strip.h"
#include "stdbool.h"
#include "esp_err.h"

typedef struct {
    int r;
    int g;
    int b;
    bool is_on;
} led_state_t;

httpd_handle_t start_webserver(led_strip_t *led);
esp_err_t led_update_state(int r, int g, int b);
esp_err_t led_set_power(bool on);
bool led_get_power(void);

#endif
