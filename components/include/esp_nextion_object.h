#pragma once

#include "esp_nextion_display.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        uint8_t page_id;
        uint8_t component_id;
        const char *name;
    } nextion_descriptor_t;  

    typedef struct
    {
        nextion_display_t *display;
        nextion_descriptor_t descriptor;
        bool touchable;
    }nextion_object_t;
    
    esp_err_t esp_nextion_object_fill(nextion_object_t *object, nextion_display_t *display, nextion_descriptor_t *descriptor, bool touchable);
    
#ifdef __cplusplus
}
#endif