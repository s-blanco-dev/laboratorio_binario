#ifndef WEB_UTILS
#define WEB_UTILS

#include <stdint.h>
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_log.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[] asm("_binary_style_css_end");

extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");

typedef struct {
  int r;
  int g;
  int b;
} led_state_t;

// la idea por ahora es, cada vez que se actualice el led en la web, se
// actualice current_led
static led_state_t current_led = {.r = 0, .g = 0, .b = 0};

static httpd_handle_t start_webserver(void);
static esp_err_t root_get_handler(httpd_req_t *req);
static esp_err_t led_post_handler(httpd_req_t *req);
static httpd_handle_t start_webserver(void);

#endif // !WEB_UTILS
