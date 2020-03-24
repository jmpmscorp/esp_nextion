#pragma once

#include "nextion_object.h"

#include "esp_err.h"

#include "nextion_object.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct nextion_crop_img nextion_crop_img_t;

    struct nextion_crop_img
    {
        nextion_object_t parent;
        nextion_err_t (*set_picc)(const void *crop_img, uint32_t number);
    };

    nextion_crop_img_t *nextion_crop_img_init(const nextion_display_t *display, nextion_descriptor_t *descriptor);
#ifdef __cplusplus
}
#endif