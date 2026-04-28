#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdint.h>
#include <stdbool.h>

#define TOUCHPAD_BTN_PHOTO      0
#define TOUCHPAD_BTN_PLAY       1
#define TOUCHPAD_BTN_NETWORK    2
#define TOUCHPAD_BTN_RECORD     3
#define TOUCHPAD_BTN_VOLUP      4
#define TOUCHPAD_BTN_VOLDOWN    5
#define TOUCHPAD_NUM_BUTTONS    6

/**
 * @brief
 */
void touchpad_init(void);

/**
 * @brief Poll whether a specific button is currently pressed.
 *
 * @param button_index  One of the TOUCHPAD_BTN_* constants (0–5).
 * @return true  if the button is being touched right now.
 * @return false if not touched, or if button_index is out of range.
 */
bool touchpad_is_pressed(uint8_t button_index);

#endif // !TOUCHPAD_H
