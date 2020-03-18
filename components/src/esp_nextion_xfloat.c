#include "esp_nextion_xfloat.h"
#include "esp_nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Xfloat]";

nextion_xfloat_t *esp_nextion_xfloat_init(nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_xfloat_t *xfloat = calloc(1, sizeof(nextion_xfloat_t));

    if(!xfloat)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_nextion_object_fill(&xfloat->parent, display, descriptor, true));
    xfloat->set_value = esp_nextion_object_set_number_value;

    return xfloat;
}