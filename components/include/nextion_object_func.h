#pragma once

#include "esp_err.h"
#include "nextion_object.h"

#ifdef __cplusplus
extern "C"
{
#endif

    nextion_err_t nextion_object_set_number_value(const void * object, int32_t value);
    nextion_err_t nextion_object_set_text(const void *object, const char *text);
    nextion_err_t nextion_object_set_picc(const void *object, uint32_t number);

#ifdef __cplusplus
}
#endif