#include "touchpad.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#define TOUCH_THRESHOLD_COEFF 0.10f

static const char *TAG = "touchpad";

static const touch_pad_t s_pads[TOUCHPAD_NUM_BUTTONS] = {
    TOUCHPAD_BTN_PHOTO_NUM,   // index 0 - PHOTO
    TOUCHPAD_BTN_PLAY_NUM,    // index 1 - PLAY
    TOUCHPAD_BTN_RECORD_NUM,  // index 2 - RECORD
    TOUCHPAD_BTN_NETWORK_NUM, // index 3 - NETWORK no anda la puta que lo pario
    TOUCHPAD_BTN_VOLUP_NUM,   // index 4 - VOL_UP
    TOUCHPAD_BTN_VOLDOWN_NUM, // index 5 - VOL_DOWN
    TOUCHPAD_BTN_GUARD_NUM,   // index 6 - GUARD
};
static uint32_t s_baseline[TOUCHPAD_NUM_BUTTONS];
static uint32_t s_threshold[TOUCHPAD_NUM_BUTTONS];

void touchpad_init(void) {
    ESP_ERROR_CHECK(touch_pad_init());
    ESP_ERROR_CHECK(touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER));

    /* configure */
    for (int i = 0; i < TOUCHPAD_NUM_BUTTONS; i++) {
        ESP_LOGI(TAG, "Configuring pad %d with touch number %d", i, s_pads[i]);
        ESP_ERROR_CHECK(touch_pad_config(s_pads[i]));
    }
    ESP_ERROR_CHECK(touch_pad_config(TOUCHPAD_BTN_GUARD_NUM));

    /* denoise */
    touch_pad_denoise_t denoise = {
        .grade = TOUCH_PAD_DENOISE_BIT4,
        .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
    };
    ESP_ERROR_CHECK(touch_pad_denoise_set_config(&denoise));
    ESP_ERROR_CHECK(touch_pad_denoise_enable());

    /* Por las dudas I: Waterproof — simplified for testing */
    touch_pad_waterproof_t waterproof = {
        .guard_ring_pad = TOUCHPAD_BTN_GUARD_NUM,
    };
    ESP_ERROR_CHECK(touch_pad_waterproof_set_config(&waterproof));
    ESP_ERROR_CHECK(touch_pad_waterproof_enable());

    /* Por las dudas II: IIR-16 filter + IIR-2 smoother */
    touch_filter_config_t filter = {
        .mode = TOUCH_PAD_FILTER_IIR_16,
        .debounce_cnt = 1,
        .noise_thr = 0,
        .jitter_step = 4,
        .smh_lvl = TOUCH_PAD_SMOOTH_IIR_2,
    };
    ESP_ERROR_CHECK(touch_pad_filter_set_config(&filter));
    ESP_ERROR_CHECK(touch_pad_filter_enable());

    ESP_ERROR_CHECK(touch_pad_fsm_start());

    /* mmmmmm */
    esp_rom_delay_us(200000);

    /* calibra */
    for (int i = 0; i < TOUCHPAD_NUM_BUTTONS; i++) {
        ESP_ERROR_CHECK(touch_pad_read_benchmark(s_pads[i], &s_baseline[i]));
        s_threshold[i] = (uint32_t)((float)s_baseline[i] * TOUCH_THRESHOLD_COEFF);
        ESP_ERROR_CHECK(touch_pad_set_thresh(s_pads[i], s_threshold[i]));
        ESP_LOGI(TAG, "Pad[%d] touch_num=%d baseline=%" PRIu32 " threshold=%" PRIu32, i, s_pads[i], s_baseline[i],
                 s_threshold[i]);
    }
}

bool touchpad_is_pressed(uint8_t btn_index) {
    if (btn_index >= TOUCHPAD_NUM_BUTTONS) {
        return false;
    }

    // /* skip cursed button */
    // if (btn_index == TOUCHPAD_BTN_NETWORK) {
    //     return false;
    // }

    uint32_t raw = 0;
    ESP_ERROR_CHECK(touch_pad_read_raw_data(s_pads[btn_index], &raw));

    if (raw <= s_baseline[btn_index]) {
        return false;
    }
    return (raw - s_baseline[btn_index]) > s_threshold[btn_index];
}

#include <inttypes.h>

void touchpad_debug_print(void) {
    for (int i = 0; i < TOUCHPAD_NUM_BUTTONS; i++) {
        uint32_t raw = 0;
        touch_pad_read_raw_data(s_pads[i], &raw);

        uint32_t diff = 0;
        bool pressed = false;

        // aumenta los valores
        if (raw > s_baseline[i]) {
            diff = raw - s_baseline[i];
            pressed = diff > s_threshold[i];
        }

        printf("Button %d (TOUCH%d): raw=%5" PRIu32 " base=%5" PRIu32 " diff=%4" PRIu32 " -> %s\n", i, s_pads[i], raw,
               s_baseline[i], diff, pressed ? "TOUCHED!" : "---");
    }
    printf("========================================\n");
}
