#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "cJson.h" // novedad
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "rgb_led.h" // novedad

#define AP_SSID "ESP32_AP"
#define AP_PASSWD "embedded" // dejar vacio para red abierta

#define SSID "caliope"
#define PASSWD "sinlugar"

#define MAX_RETRY 5

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[]   asm("_binary_style_css_end");

extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[]   asm("_binary_app_js_end");

/* ======== */

static void wifi_start_ap(void);
static void wifi_start_sta(void);
static httpd_handle_t start_webserver(void);
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

  wifi_start_ap();
  // wifi_start_sta();
  start_webserver();
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
              .max_connection = 1,
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

// == Ejercicio 3 ===
/**
 * @brief Atiende las peticiones HTTP GET realizadas a la ruta raíz del
 * servidor.
 *
 * Esta función actúa como handler de la URI "/". Se ejecuta automáticamente
 * cuando un navegador o cliente HTTP accede a la dirección principal del
 * ESP32, por ejemplo: http://192.168.4.1/.
 *
 * En esta primera implementación responde con un texto simple, permitiendo
 * verificar que el servidor HTTP embebido está funcionando correctamente.
 *
 * @param req Puntero a la estructura que representa la petición HTTP
 * recibida.
 *
 * @return ESP_OK si la respuesta fue enviada correctamente.
 */
static esp_err_t root_get_handler(httpd_req_t *req) {
  const char *response = "Ciao Mondo";
  // el hanlder responde a la preticion del navegador con el texto de arriba
  httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

/**
 * @brief Define la URI raiz del servidor HTTP.
 *
 * Asocia la ruta "/" con el metodo HTTP GET y con la fucnion
 * root_get_handler(). Cuando un cliente entra a la pagina principal del
 * ESP32, el servidor ejecuta este handler para generar la respuesta.
 */
static const httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL, // *Abdul Bari reference
};

/**
 * @brief Inicializa y arranca el servidor HTTP embebido del ESP32.
 *
 * Esta función crea una configuración por defecto para el servidor HTTP
 * usando HTTPD_DEFAULT_CONFIG(), inicia el servidor mediante httpd_start() y
 * registra los handlers de las rutas que el servidor debe atender.
 *
 * En esta etapa del laboratorio se registra únicamente la ruta raíz "/",
 * asociada al método HTTP GET. Cuando un navegador accede a dicha ruta, se
 * ejecuta root_get_handler(), que devuelve una respuesta simple de prueba.
 *
 * El servidor se levanta después de inicializar el WiFi en modo AP, de forma
 * que un dispositivo conectado a la red generada por el ESP32 pueda acceder
 * desde un navegador a la dirección IP del punto de acceso.
 *
 * @return Handle del servidor HTTP si se inició correctamente, o NULL si no
 * pudo iniciarse.
 */
static httpd_handle_t start_webserver(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_register_uri_handler(server, &root_uri);
    // agrego la uri para POST / led
    httpd_register_uri_handler(server, &led_post_uri);

    ESP_LOGI("WEB", "Servidor HTTP iniciado");
  } else {
    ESP_LOGE("WEB", "Error capital al iniciar el servidor HTTP");
  }
  return server;
}

typedef struct{
  int r;
  int g;
  int b;
} led_state_t;

// la idea por ahora es, cada vez que se actualice el led en la web, se actualice current_led
static led_state_t current_led = {
  .r = 0;
  .g = 0;
  .b = 0
}

  /**
 * @brief Atiende las peticiones HTTP POST enviadas a la ruta "/led".
 *
 * Esta función se ejecuta cuando la página web envía un comando al ESP32
 * para cambiar el color del LED RGB. El navegador manda en el cuerpo de la
 * petición un objeto JSON con los valores de rojo, verde y azul, por ejemplo:
 *
 * { "r": 255, "g": 0, "b": 128 }
 *
 * El handler lee el cuerpo de la petición HTTP, lo interpreta usando cJSON,
 * extrae los valores numéricos de los campos "r", "g" y "b", actualiza la
 * variable global que guarda el estado actual del LED y luego llama a la
 * función correspondiente de la librería rgb_led para aplicar físicamente
 * ese color en la placa.
 *
 * Si el JSON recibido es inválido o faltan campos, responde con un error HTTP.
 * Si todo sale correctamente, responde al navegador con un JSON simple
 * indicando que la operación fue realizada.
 *
 * @param req Puntero a la estructura que representa la petición HTTP recibida.
 *
 * @return ESP_OK si el color fue recibido y procesado correctamente.
 *         ESP_FAIL si ocurrió un error al recibir o parsear el JSON.
 */
static esp_err_t led_post_handler(httpd_req_t *req){
  char buffer[128];

  int received = httpd_req_recv(req, buffer, sizeof(buffer) -1);

  if (received <= 0){
    httpd_req_send_err(req, HTTPD_400_BAD_REQUEST, "No se recibio cuerpo JSON");
    return ESP_FAIL;
  }

  buffer[received] = '\0';
  
  cJSON *json = cJSON_Parse(buffer);

  if (json == NULL){
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON invalido");

    cJSON *r_item = cJSON_GetObjectItem(json, "r");
    cJSON *g_item = cJSON_GetObjectItem(json, "g");
    cJSON *b_item = cJSON_GetObjectItem(json, "b");

    if (!cJSON_IsNumber(r_item) || !cJSON_IsNumber(g_item) || !cJSON_IsNumber(b_item)) {
    cJSON_Delete(json);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Faltan valores RGB");
    return ESP_FAIL;
  }

  int r = r_item->valueint;
  int g = g_item->valueint;
  int b = b_item->valueint;

  current_led.r = r;
  current_led.g = g;
  current_led.b = b;

  /*
   * Aca se debe llamar a la funcion real de la libreria del led
   */

  ESP_LOGI(TAG, "LED actualizado: R=%d, G=%d, B=%d", r, g, b);

  cJSON_Delete(json);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

// uri para POST / led
static const httpd_uri_t led_post_uri = {
  .uri = "/led",
  .method = HTTP_POST,
  .handler = led_post_handler,
  .user_ctx = NULL,
};

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
