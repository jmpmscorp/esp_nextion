#include "nextion_dual_state_button.h"
#include "nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Dual St Button]";

nextion_dual_state_button_t *nextion_dual_state_button_init(const nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_dual_state_button_t *dsbtn = calloc(1, sizeof(nextion_dual_state_button_t));

    if(!dsbtn)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(nextion_object_fill(&dsbtn->parent, display, descriptor, true));
    dsbtn->set_value = nextion_object_set_number_value;

    return dsbtn;
}