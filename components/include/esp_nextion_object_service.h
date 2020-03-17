#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t esp_nextion_object_set_value(void * obj, uint32_t value);

#ifdef __cplusplus
}
#endif