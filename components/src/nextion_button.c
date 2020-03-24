#include "nextion_button.h"


#include "nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Button]";

nextion_button_t *nextion_button_init(const nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_button_t *btn = calloc(1, sizeof(nextion_button_t));

    if(!btn)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(nextion_object_fill(&btn->parent, display, descriptor, true));
    btn->set_text = nextion_object_set_text;

    return btn;
}