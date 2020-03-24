#include "nextion_text.h"

#include "nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Text]";

nextion_text_t *nextion_text_init(const nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_text_t *text_obj = calloc(1, sizeof(nextion_text_t));

    if(!text_obj)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(nextion_object_fill(&text_obj->parent, display, descriptor, true));
    text_obj->set_text = nextion_object_set_text;

    return text_obj;
}