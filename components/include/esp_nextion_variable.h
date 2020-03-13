#pragma once

#include "esp_nextion_display.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        esp_err_t (*set_value)(nextion_display_t *display, uint32_t value);
        esp_err_t (*get_value)(nextion_display_t *display, uint32_t *value);
    } nextion_variable_t;

#ifdef __cplusplus
}
#endif