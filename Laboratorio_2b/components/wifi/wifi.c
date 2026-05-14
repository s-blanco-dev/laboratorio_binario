#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include <stdint.h>
#include <string.h>

static const char *TAG = "WIFI";

static uint8_t s_retry_num = 0;
static const uint8_t MAX_RETRIES = 5;

/**
 * @brief Inicia el ESP32 en modo AP.
 *
 * Crea una red WiFi con el SSID y contraseña indicados.
 * Si la contraseña está vacía, la red se configura como abierta.
 *
 * @param ap_ssid Nombre de la red WiFi.
 * @param ap_passwd Contraseña de la red WiFi.
 */
void wifi_start_ap(const char *ap_ssid, const char *ap_passwd) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    /* ahorasi */
    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.ap.ssid, ap_ssid, strnlen(ap_ssid, sizeof(wifi_config.ap.ssid) - 1));
    memcpy(wifi_config.ap.password, ap_passwd, strnlen(ap_passwd, sizeof(wifi_config.ap.password) - 1));

    wifi_config.ap.ssid_len = (uint8_t)strnlen(ap_ssid, sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.max_connection = 10;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.channel = 1;

    // Si no hay password, se crea una red abierta
    if (strlen(ap_passwd) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "AP inicializado. SSID: %s", ap_ssid);
}

/**
 * @brief Manejador de eventos WiFi/IP en modo STA.
 *
 * Se encarga de reconectar si se pierde conexión y de mostrar la IP obtenida.
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRIES) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Reintentando conexion...");
        } else {
            ESP_LOGI(TAG, "No se pudo conectar al AP");
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

/**
 * @brief Conecta el ESP32 a una red WiFi en modo STA.
 *
 * Configura SSID/contraseña, registra eventos y arranca el WiFi.
 *
 * @param ssid Nombre de la red WiFi.
 * @param passwd Contraseña de la red WiFi.
 */
void wifi_start_sta(const char *ssid, const char *passwd) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.sta.ssid, ssid, strnlen(ssid, sizeof(wifi_config.sta.ssid) - 1));
    memcpy(wifi_config.sta.password, passwd, strnlen(passwd, sizeof(wifi_config.sta.password) - 1));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
