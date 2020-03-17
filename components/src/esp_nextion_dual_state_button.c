#include "esp_nextion_dual_state_button.h"
#include "esp_nextion_object_service.h"

#include "esp_log.h"

static const char * TAG = "[Nex Dual St Button]";

typedef struct 
{
    nextion_object_t * parent;
    nextion_dual_state_button_t * button;
};

nextion_dual_state_button_t *esp_nextion_dual_state_button_init(nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_dual_state_button_t *dsbtn = calloc(1, sizeof(nextion_dual_state_button_t));

    if(!dsbtn)
    {
        ESP_LOGE(TAG, "Can't allocate button");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_nextion_object_init(&dsbtn->parent, display, descriptor, true));
    dsbtn->set_value = esp_nextion_object_set_value;

    return dsbtn;
}