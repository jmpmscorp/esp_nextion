#include "nextion_variable.h"

#include "nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Variable]";

nextion_variable_t *nextion_variable_init(nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_variable_t *var = calloc(1, sizeof(nextion_variable_t));

    if(!var)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(nextion_object_fill(&var->parent, display, descriptor, true));
    var->set_number_value = nextion_object_set_number_value;

    return var;
}