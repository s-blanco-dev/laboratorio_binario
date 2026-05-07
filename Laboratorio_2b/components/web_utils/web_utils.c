#include "esp_err.h"
#include "esp_http_server.h"
#include "web_utils.h"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "weballs";

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
esp_err_t root_get_handler(httpd_req_t *req) {
  size_t index_html_size = index_html_end - index_html_start;
  httpd_resp_send(req, (const char *)index_html_start, index_html_size);
  return ESP_OK;
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
esp_err_t led_post_handler(httpd_req_t *req) {
  char buffer[128];

  int received = httpd_req_recv(req, buffer, sizeof(buffer) - 1);

  if (received <= 0) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  buffer[received] = '\0';

  cJSON *json = cJSON_Parse(buffer);

  if (json == NULL) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON invalido");

    cJSON *r_item = cJSON_GetObjectItem(json, "r");
    cJSON *g_item = cJSON_GetObjectItem(json, "g");
    cJSON *b_item = cJSON_GetObjectItem(json, "b");

    if (!cJSON_IsNumber(r_item) || !cJSON_IsNumber(g_item) ||
        !cJSON_IsNumber(b_item)) {
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
  return ESP_OK;
}
// uri para POST / led
static const httpd_uri_t led_post_uri = {
    .uri = "/led",
    .method = HTTP_POST,
    .handler = led_post_handler,
    .user_ctx = NULL,
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
 * pudo iniciarse. */
httpd_handle_t start_webserver(void) {
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
