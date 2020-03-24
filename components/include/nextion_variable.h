#pragma once

#include "nextion_object.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        nextion_object_t parent;
        nextion_err_t (*set_number_value)(const void *variable, int32_t value);
        nextion_err_t (*get_number_value)(const void *variable, uint32_t *value);
        nextion_err_t (*set_string_value)(const void *variable, const char *value);
        nextion_err_t (*get_string_value)(const void *variable, const char *value_buffer, size_t size);
    } nextion_variable_t;

    nextion_variable_t *nextion_variable_init(const nextion_display_t *display, nextion_descriptor_t *descriptor);

#ifdef __cplusplus
}
#endif