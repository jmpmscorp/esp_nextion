#pragma once

#include "nextion_object.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct nextion_dual_state_button
    {
        nextion_object_t parent;
        // esp_err_t (*set_text)(nextion_display_t *display, const char *text);
        // esp_err_t (*get_text)(nextion_display_t *display, char *text_buffer, size_t size);
        nextion_err_t (*set_value)(const void *dsbtn, int32_t value);
        // esp_err_t (*get_value)(nextion_display_t *display, uint32_t *value);
        // esp_err_t (*set_background_color_bco)(nextion_display_t *display, uint32_t color);
        // esp_err_t (*get_background_color_bco)(nextion_display_t *display, uint32_t *color);
        // esp_err_t (*set_background_color_bco2)(nextion_display_t *display, uint32_t color);
        // esp_err_t (*get_background_color_bco2)(nextion_display_t *display, uint32_t *color);
        // esp_err_t (*set_font_color_pco)(nextion_display_t *display, uint32_t color);
        // esp_err_t (*get_font_color_pco)(nextion_display_t *display, uint32_t *color);
        // esp_err_t (*set_font_color_pco2)(nextion_display_t *display, uint32_t color);
        // esp_err_t (*get_font_color_pco2)(nextion_display_t *display, uint32_t *color);
        // esp_err_t (*set_place_xcen)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_place_xcen)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_place_ycen)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_place_ycen)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_font)(nextion_display_t *display, uint32_t font_id);
        // esp_err_t (*get_font)(nextion_display_t *display, uint32_t *font_id);
        // esp_err_t (*set_background_crop_picc)(nextion_display_t *display, uint32_t pic_id);
        // esp_err_t (*get_background_crop_picc)(nextion_display_t *display, uint32_t *pic_id);
        // esp_err_t (*set_background_crop_picc2)(nextion_display_t *display, uint32_t pic_id);
        // esp_err_t (*get_background_crop_picc2)(nextion_display_t *display, uint32_t *pic_id);
        // esp_err_t (*set_background_image_pic)(nextion_display_t *display, uint32_t pic_id);
        // esp_err_t (*get_background_image_pic)(nextion_display_t *display, uint32_t *pic_id);
        // esp_err_t (*set_background_image_pic2)(nextion_display_t *display, uint32_t pic_id);
        // esp_err_t (*get_background_image_pic2)(nextion_display_t *display, uint32_t *pic_id);
        // esp_err_t (*set_wordwrap_isbr)(nextion_display_t *display, bool value);
        // esp_err_t (*get_wordwrap_isbr)(nextion_display_t *display, bool *value);
    }nextion_dual_state_button_t;

    nextion_dual_state_button_t *nextion_dual_state_button_init(const nextion_display_t *display, nextion_descriptor_t *descriptor);

#ifdef __cplusplus
}
#endif