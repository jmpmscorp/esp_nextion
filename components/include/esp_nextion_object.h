#pragma once

#include "esp_nextion_display.h"
#include "esp_nextion_button.h"
#include "esp_nextion_dual_state_button.h"
#include "esp_nextion_number.h"
#include "esp_nextion_text.h"
#include "esp_nextion_variable.h"
#include "esp_nextion_xfloat.h"

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

    
    nextion_button_t *esp_nextion_button_init(nextion_descriptor_t *descriptor);
    nextion_dual_state_button_t *esp_nextion_dual_state_button_init(nextion_descriptor_t *descriptor);
    nextion_number_t *esp_nextion_number_init(nextion_descriptor_t *descriptor);
    nextion_text_t *esp_nextion_text_init(nextion_descriptor_t *descriptor);
    nextion_variable_t *esp_nextion_variable_init(nextion_descriptor_t *descriptor);
    nextion_xfloat_t *esp_nextion_xfloat_init(nextion_descriptor_t *descriptor);


#ifdef __cplusplus
}
#endif