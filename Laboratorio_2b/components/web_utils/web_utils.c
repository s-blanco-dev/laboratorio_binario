#include "esp_err.h"
#include "esp_http_server.h"
#include "web_utils.h"
#include "cJSON.h"
#include "esp_log.h"
#include "rgb_led.h"
#include "led_strip.h"

static const char *TAG = "weballs";

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
    bool is_on;
} led_state_t;

static led_state_t current_led = {
    .r = 0,
    .g = 0,
    .b = 0,
    .is_on = false,
};

// Puntero global privado al LED fisico inicializado en main.
static led_strip_t *web_led = NULL;

static esp_err_t root_get_handler(httpd_req_t *req);
static esp_err_t style_get_handler(httpd_req_t *req);
static esp_err_t app_js_get_handler(httpd_req_t *req);
static esp_err_t led_post_handler(httpd_req_t *req);
static esp_err_t led_get_handler(httpd_req_t *req);

/**
 * @brief Actualiza el estado logico del LED y aplica el color fisicamente.
 *
 * Esta funcion centraliza los cambios de color del LED. Debe ser usada tanto
 * por el handler POST /led como por la logica del TouchPad, para asegurar que
 * el estado devuelto por GET /led siempre coincida con el color real del LED.
 *
 * @param r Componente roja del color.
 * @param g Componente verde del color.
 * @param b Componente azul del color.
 *
 * @return ESP_OK si el LED fue actualizado correctamente, ESP_FAIL en caso de error.
 */
esp_err_t led_update_state(int r, int g, int b);
esp_err_t led_set_power(bool on);

/**
 * @brief Define la URI raiz del servidor HTTP.
 */
static const httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t style_uri = {
    .uri = "/style.css",
    .method = HTTP_GET,
    .handler = style_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t app_js_uri = {
    .uri = "/app.js",
    .method = HTTP_GET,
    .handler = app_js_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t led_post_uri = {
    .uri = "/led",
    .method = HTTP_POST,
    .handler = led_post_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t led_get_uri = {
    .uri = "/led",
    .method = HTTP_GET,
    .handler = led_get_handler,
    .user_ctx = NULL,
};

/**
 * @brief Atiende las peticiones HTTP GET realizadas a la ruta raiz "/".
 */
static esp_err_t root_get_handler(httpd_req_t *req) {
    size_t index_html_size = index_html_end - index_html_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_size);

    return ESP_OK;
}

/**
 * @brief Atiende las peticiones HTTP GET realizadas a "/style.css".
 */
static esp_err_t style_get_handler(httpd_req_t *req) {
    size_t style_css_size = style_css_end - style_css_start;

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)style_css_start, style_css_size);

    return ESP_OK;
}

/**
 * @brief Atiende las peticiones HTTP GET realizadas a "/app.js".
 */
static esp_err_t app_js_get_handler(httpd_req_t *req) {
    size_t app_js_size = app_js_end - app_js_start;

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_js_start, app_js_size);

    return ESP_OK;
}

/**
 * @brief Atiende las peticiones HTTP POST enviadas a "/led".
 *
 * Recibe un JSON con valores RGB:
 *
 * { "r": 255, "g": 0, "b": 128 }
 *
 * Luego actualiza el LED fisico y el estado interno usado por GET /led.
 */
static esp_err_t led_post_handler(httpd_req_t *req) {
    char buffer[128];

    int received = httpd_req_recv(req, buffer, sizeof(buffer) - 1);

    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No se recibio body");
        return ESP_FAIL;
    }

    buffer[received] = '\0';

    cJSON *json = cJSON_Parse(buffer);

    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON invalido");
        return ESP_FAIL;
    }

    cJSON *r_item = cJSON_GetObjectItem(json, "r");
    cJSON *g_item = cJSON_GetObjectItem(json, "g");
    cJSON *b_item = cJSON_GetObjectItem(json, "b");

    if (!cJSON_IsNumber(r_item) ||
        !cJSON_IsNumber(g_item) ||
        !cJSON_IsNumber(b_item)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Faltan valores RGB");
        return ESP_FAIL;
    }

    int r = r_item->valueint;
    int g = g_item->valueint;
    int b = b_item->valueint;

    cJSON_Delete(json);

    esp_err_t err = led_update_state(r, g, b);

    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error al actualizar LED");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

/**
 * @brief Atiende las peticiones HTTP GET realizadas a "/led".
 *
 * Devuelve el estado actual del LED en formato JSON.
 */
static esp_err_t led_get_handler(httpd_req_t *req) {
    char response[96];

    snprintf(
        response,
        sizeof(response),
        "{\"r\":%d,\"g\":%d,\"b\":%d,\"on\":%s}",
        current_led.r,
        current_led.g,
        current_led.b,
        current_led.is_on ? "true" : "false"
    );

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

/**
 * @brief Actualiza el estado del LED y aplica el color fisico.
 */

esp_err_t led_update_state(int r, int g, int b) {
    if (web_led == NULL) {
        ESP_LOGE(TAG, "LED no inicializado");
        return ESP_FAIL;
    }

    current_led.r = r;
    current_led.g = g;
    current_led.b = b;

    /*
     * Si el LED está apagado, solo guardamos el color lógico.
     * No actualizamos el hardware porque eso lo prendería visualmente.
     */
    if (!current_led.is_on) {
        ESP_LOGI(TAG, "Color guardado con LED apagado: R=%d, G=%d, B=%d", r, g, b);
        return ESP_OK;
    }

    color_t color = {
        .r = r,
        .g = g,
        .b = b,
    };

    esp_err_t err = led_set_color(web_led, color);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo actualizar el LED");
        return err;
    }

    ESP_LOGI(TAG, "LED actualizado: R=%d, G=%d, B=%d", r, g, b);

    return ESP_OK;
}

esp_err_t led_set_power(bool on) {
    if (web_led == NULL) {
        ESP_LOGE(TAG, "LED no inicializado");
        return ESP_FAIL;
    }

    current_led.is_on = on;

    if (!on) {
        ESP_LOGI(TAG, "LED apagado");
        return led_off(web_led);
    }

    color_t color = {
        .r = current_led.r,
        .g = current_led.g,
        .b = current_led.b,
    };

    ESP_LOGI(
        TAG,
        "LED encendido con ultimo color: R=%d, G=%d, B=%d",
        current_led.r,
        current_led.g,
        current_led.b
    );

    return led_set_color(web_led, color);
}

/**
 * @brief Inicializa y arranca el servidor HTTP embebido del ESP32.
 *
 * Registra los handlers para servir la pagina web, los archivos estaticos
 * y los endpoints GET /led y POST /led.
 *
 * @param led Puntero al LED RGB inicializado en main.
 *
 * @return Handle del servidor HTTP si se inicio correctamente, o NULL si fallo.
 */
httpd_handle_t start_webserver(led_strip_t *led) {
    web_led = led;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &style_uri);
        httpd_register_uri_handler(server, &app_js_uri);
        httpd_register_uri_handler(server, &led_get_uri);
        httpd_register_uri_handler(server, &led_post_uri);

        ESP_LOGI("WEB", "Servidor HTTP iniciado");
    } else {
        ESP_LOGE("WEB", "Error al iniciar el servidor HTTP");
    }

    return server;
}
