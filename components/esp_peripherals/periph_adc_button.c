/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "esp_log.h"
#include "audio_error.h"
#include "periph_adc_button.h"

static const char *TAG = "PERIPH_ADC_BUTTON";

typedef struct {
    int adc_channels;
    adc_btn_list *list;
} periph_adc_btn_t;

static void btn_cb(void *user_data, int adc, int id, btn_state state)
{
    esp_periph_handle_t self = (esp_periph_handle_t)user_data;
    periph_adc_button_event_id_t event_id = PERIPH_ADC_BUTTON_IDLE;
    if (state == BTN_STATE_CLICK) {
        event_id = PERIPH_ADC_BUTTON_CLICK;
    } else if (state == BTN_STATE_PRESS) {
        event_id = PERIPH_ADC_BUTTON_PRESS;
    } else if (state == BTN_STATE_RELEASE) {
        event_id = PERIPH_ADC_BUTTON_RELEASE;
    }
    //Send ADC as data and ID as data_len
    esp_periph_send_event(self, event_id, (void *)adc, id);
}

static esp_err_t _adc_button_destroy(esp_periph_handle_t self)
{
    periph_adc_btn_t *periph_adc_btn = esp_periph_get_data(self);
    adc_btn_destroy_list(periph_adc_btn->list);
    free(periph_adc_btn);
    return ESP_OK;
}

static esp_err_t _adc_button_init(esp_periph_handle_t self)
{
    periph_adc_btn_t *periph_adc_btn = esp_periph_get_data(self);
    adc_btn_init((void *)self, btn_cb, periph_adc_btn->list);
    return ESP_OK;
}

esp_periph_handle_t periph_adc_button_init(periph_adc_button_cfg_t *config)
{
    esp_periph_handle_t periph = esp_periph_create(PERIPH_ID_ADC_BTN, "periph_adc_btn");
    AUDIO_MEM_CHECK(TAG, periph, return NULL);

    periph_adc_btn_t *periph_adc_btn = calloc(1, sizeof(periph_adc_btn_t));
    AUDIO_MEM_CHECK(TAG, periph_adc_btn, {
        free(periph);
        return NULL;
    });
    periph_adc_btn->adc_channels = config->arr_size;
    periph_adc_btn->list = adc_btn_create_list(config->arr, config->arr_size);
    AUDIO_MEM_CHECK(TAG, periph_adc_btn->list, {
        free(periph);
        free(periph_adc_btn);
        return NULL;
    });

    esp_periph_set_data(periph, periph_adc_btn);
    esp_periph_set_function(periph, _adc_button_init, NULL, _adc_button_destroy);
    return periph;
}
