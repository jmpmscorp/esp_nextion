#pragma once

#include "esp_nextion_display.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        esp_err_t (*set_text)(nextion_display_t *display, const char *text);
        esp_err_t (*get_text)(nextion_display_t *display, char *text_buffer, size_t size);
        esp_err_t (*set_background_color_bco)(nextion_display_t *display, uint32_t color);
        esp_err_t (*get_background_color_bco)(nextion_display_t *display, uint32_t *color);
        esp_err_t (*set_background_color_bco2)(nextion_display_t *display, uint32_t color);
        esp_err_t (*get_background_color_bco2)(nextion_display_t *display, uint32_t *color);
        esp_err_t (*set_font_color_pco)(nextion_display_t *display, uint32_t color);
        esp_err_t (*get_font_color_pco)(nextion_display_t *display, uint32_t *color);
        esp_err_t (*set_font_color_pco2)(nextion_display_t *display, uint32_t color);
        esp_err_t (*get_font_color_pco2)(nextion_display_t *display, uint32_t *color);
        esp_err_t (*set_place_xcen)(nextion_display_t *display, uint32_t number);
        esp_err_t (*get_place_xcen)(nextion_display_t *display, uint32_t *number);
        esp_err_t (*set_place_ycen)(nextion_display_t *display, uint32_t number);
        esp_err_t (*get_place_ycen)(nextion_display_t *display, uint32_t *number);
        esp_err_t (*set_font)(nextion_display_t *display, uint32_t font_id);
        esp_err_t (*get_font)(nextion_display_t *display, uint32_t *font_id);
        esp_err_t (*set_background_crop_picc)(nextion_display_t *display, uint32_t pic_id);
        esp_err_t (*get_background_crop_picc)(nextion_display_t *display, uint32_t *pic_id);
        esp_err_t (*set_background_crop_picc2)(nextion_display_t *display, uint32_t pic_id);
        esp_err_t (*get_background_crop_picc2)(nextion_display_t *display, uint32_t *pic_id);
        esp_err_t (*set_background_image_pic)(nextion_display_t *display, uint32_t pic_id);
        esp_err_t (*get_background_image_pic)(nextion_display_t *display, uint32_t *pic_id);
        esp_err_t (*set_background_image_pic2)(nextion_display_t *display, uint32_t pic_id);
        esp_err_t (*get_background_image_pic2)(nextion_display_t *display, uint32_t *pic_id);
        esp_err_t (*set_wordwrap_isbr)(nextion_display_t *display, bool value);
        esp_err_t (*get_wordwrap_isbr)(nextion_display_t *display, bool *value);
    } nextion_button_t;

#ifdef __cplusplus
}
#endif