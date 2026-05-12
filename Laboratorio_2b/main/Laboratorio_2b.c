#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "web_utils.h"
#include "led_strip.h"
#include "touchpad.h"
#include "delay.h"

#define AP_SSID "ESP32_AP"
#define AP_PASSWD "embedded" // dejar vacio para red abierta

#define SSID "caliope"
#define PASSWD "sinlugar"

#define MAX_RETRY 5

/* ======== */

static void wifi_start_ap(void);
// static void wifi_start_sta(void);
static const char *TAG = "alfredo arno bot";
static int s_retry_num = 0;

/* ======== */

void app_main(void) {
  esp_err_t ret = nvs_flash_init();

  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);

  led_strip_t *led = NULL;
  ESP_ERROR_CHECK(led_rgb_init(&led));

  touchpad_init();

  wifi_start_ap();
  // wifi_start_sta();
  start_webserver(led);

  led_update_state(0, 0, 0);

  bool photo_was_pressed = false;
  bool network_was_pressed = false;
  bool record_was_pressed = false;
  bool play_was_pressed = false;

  while (1) {
    bool photo_pressed = touchpad_is_pressed(TOUCHPAD_BTN_PHOTO);
    bool network_pressed = touchpad_is_pressed(TOUCHPAD_BTN_NETWORK);
    bool record_pressed = touchpad_is_pressed(TOUCHPAD_BTN_RECORD);
    bool play_pressed = touchpad_is_pressed(TOUCHPAD_BTN_PLAY);

    if (photo_pressed && !photo_was_pressed) {
      // PHOTO: naranja
      led_update_state(255, 165, 0);
    }

    if (network_pressed && !network_was_pressed) {
      // NETWORK: magenta
      led_update_state(255, 0, 255);
    }

    if (record_pressed && !record_was_pressed) {
      // RECORD: apagado
      led_update_state(0, 0, 0);
    }

    if (play_pressed && !play_was_pressed) {
      // PLAY: blanco
      led_update_state(255, 255, 255);
    }

    photo_was_pressed = photo_pressed;
    network_was_pressed = network_pressed;
    record_was_pressed = record_pressed;
    play_was_pressed = play_pressed;

    delay_ms(50);
  }
}

/**
 * @brief Inicializa el WiFi del ESP32-S2 en modo Access Point.
 *
 * Esta función configura al ESP32-S2 para que cree su propia red WiFi.
 * Primero inicializa la infraestructura de red con esp_netif_init() y crea
 * el event loop por defecto con esp_event_loop_create_default().
 *
 * Luego crea la interfaz de red correspondiente al modo AP mediante
 * esp_netif_create_default_wifi_ap(), inicializa el driver WiFi y carga
 * los parámetros de configuración del Access Point: SSID, contraseña,
 * cantidad máxima de conexiones, canal y modo de autenticación.
 *
 * Si la contraseña definida está vacía, se cambia automáticamente el modo
 * de autenticación a WIFI_AUTH_OPEN, permitiendo una red abierta.
 *
 * En este modo, el ESP32-S2 actúa como punto de acceso local. Por defecto,
 * queda disponible en la dirección 192.168.4.1 y asigna direcciones IP a
 * los clientes conectados mediante DHCP.
 *
 * @return No retorna.
 */
static void wifi_start_ap(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_ap();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&config));

  wifi_config_t wifi_config = {
      .ap =
          {
              .ssid = AP_SSID,
              .ssid_len = strlen(AP_SSID),
              .password = AP_PASSWD,
              .max_connection = 10,
              .authmode = WIFI_AUTH_WPA_WPA2_PSK,
              .channel = 1,
              // .ssid_hidden = 1,
          },
  };
  // Si la password es vacia, entonces signfica que la red es abierta y que
  // funciona igual
  if (strlen(AP_PASSWD) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("WIFI", "AP inicializado. SSID: %s", AP_SSID);
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < MAX_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      ESP_LOGI(TAG, "connect to the AP fail");
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
  }
}

/* ======== */

static void wifi_start_sta(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = SSID,
              .password = PASSWD,
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
}
