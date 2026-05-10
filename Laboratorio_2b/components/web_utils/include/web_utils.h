#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include "esp_http_server.h"
#include "led_strip.h"

httpd_handle_t start_webserver(led_strip_t *led);


#endif
