#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t nextion_object_set_number_value(const void * object, int32_t value);
    esp_err_t nextion_object_set_text(const void *object, const char *text);

#ifdef __cplusplus
}
#endif