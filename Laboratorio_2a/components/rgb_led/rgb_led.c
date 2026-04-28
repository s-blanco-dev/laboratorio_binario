#include "../include/rgb_led.h"
#include "esp_err.h"
#include "led_strip.h"
#include <stdint.h>

/*
 * @brief Función para establecer el led en un color RGB especificado
 *
 * Nota: El driver led_strip_rmt_ws2812 permite pasar por argumento un tiempo
 * límite para refrescar el led. Por defecto, le ponemos 100ms como máximo, sino
 * devuelve ESP_ERR_TIMEOUT
 *
 * @param led puntero al led_strip_t uuu
 * @param colors estructura que permite especificar una terna RGB
 *
 * @return
 *      - ESP_OK si no pasó nada feo
 *      - Otra cosa si pasó algo triste
 * */
esp_err_t led_set_color(led_strip_t *led, color_t colors) {
  esp_err_t err = led->set_pixel(led, 0, colors.r, colors.g, colors.b);
  if (err != ESP_OK) {
    return err;
  }
  esp_err_t morr = led->refresh(led, 100);
  if (morr != ESP_OK) {
    return morr;
  }
  return ESP_OK;
}

/*
 * Función wrapper a ruego del Dr. Ing. Nicolás Alberto Calarco
 * No funciona, no use. Solo valor teórico
 * */
esp_err_t create_led(led_strip_t *led) { return led_rgb_init(&led); }
