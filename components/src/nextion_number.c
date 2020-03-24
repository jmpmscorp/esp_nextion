#include "nextion_number.h"

#include "nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Number]";

nextion_number_t *nextion_number_init(const nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_number_t *number = calloc(1, sizeof(nextion_number_t));

    if(!number)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(nextion_object_fill(&number->parent, display, descriptor, true));
    number->set_value = nextion_object_set_number_value;

    return number;
}