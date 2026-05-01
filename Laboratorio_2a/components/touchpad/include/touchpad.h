#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdint.h>
#include <stdbool.h>

#define TOUCHPAD_BTN_PHOTO_NUM      TOUCH_PAD_NUM6
#define TOUCHPAD_BTN_PLAY_NUM       TOUCH_PAD_NUM2
#define TOUCHPAD_BTN_RECORD_NUM     TOUCH_PAD_NUM5
#define TOUCHPAD_BTN_NETWORK_NUM    TOUCH_PAD_NUM14
#define TOUCHPAD_BTN_VOLUP_NUM      TOUCH_PAD_NUM1
#define TOUCHPAD_BTN_VOLDOWN_NUM    TOUCH_PAD_NUM3
#define TOUCHPAD_BTN_GUARD_NUM      TOUCH_PAD_NUM4

#define TOUCHPAD_BTN_PHOTO      0
#define TOUCHPAD_BTN_PLAY       1
#define TOUCHPAD_BTN_RECORD     2
#define TOUCHPAD_BTN_NETWORK    3
#define TOUCHPAD_BTN_VOLUP      4
#define TOUCHPAD_BTN_VOLDOWN    5
#define TOUCHPAD_BTN_GUARD      6

#define TOUCHPAD_NUM_BUTTONS    7

void touchpad_init(void);
bool touchpad_is_pressed(uint8_t button_index);
void touchpad_debug_print(void);

#endif
