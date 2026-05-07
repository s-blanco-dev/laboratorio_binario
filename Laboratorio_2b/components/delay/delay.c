#include "delay.h"
#include <esp_rom_sys.h>

/*
 * @brief genera un delay expresado en milisegundos
 *
 * @param ms    El tiempo del delay en milisegundos
 * @return void
 * */
void delay_ms(uint32_t ms) {
    // us = x10^-6
    // ms = x10^-3
    esp_rom_delay_us(ms * 1000);
}

/*
 * @brief genera un delay expresado en segundos
 *
 * @param s    El tiempo del delay en segundos
 * @return void
 * */
void delay_s(uint32_t s) {
    // us = x10^-6
    // ms = x10^0
    esp_rom_delay_us(s * 1000000);
}
