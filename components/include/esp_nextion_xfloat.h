#pragma once

#include "esp_nextion_object.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        nextion_object_t parent;
        esp_err_t (*set_value)(const void *xfloat, int32_t value);
        // esp_err_t (*get_value)(nextion_display_t *display, uint32_t *value);
        // esp_err_t (*set_place_xcen)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_place_xcen)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_place_ycen)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_place_ycen)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_font)(nextion_display_t *display, uint32_t font_id);
        // esp_err_t (*get_font)(nextion_display_t *display, uint32_t *font_id);
        // esp_err_t (*set_font_color_pco)(nextion_display_t *display, uint32_t color);
        // esp_err_t (*get_font_color_pco)(nextion_display_t *display, uint32_t *color);
        // esp_err_t (*set_background_crop_picc)(nextion_display_t *display, uint32_t pic_id);
        // esp_err_t (*get_background_crop_picc)(nextion_display_t *display, uint32_t *pic_id);
        // esp_err_t (*set_spax)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_spax)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_spay)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_spay)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_float_point_rigth_vss0)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_float_point_rigth_vss0)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_float_point_left_vss1)(nextion_display_t *display, uint32_t number);
        // esp_err_t (*get_float_point_left_vss1)(nextion_display_t *display, uint32_t *number);
        // esp_err_t (*set_wordwrap_isbr)(nextion_display_t *display, bool value);
        // esp_err_t (*get_wordwrap_isbr)(nextion_display_t *display, bool *value);
    } nextion_xfloat_t;

    nextion_xfloat_t *esp_nextion_xfloat_init(nextion_display_t *display, nextion_descriptor_t *descriptor);
#ifdef __cplusplus
}
#endif