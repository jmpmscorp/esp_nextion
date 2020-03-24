#include "nextion_crop_img.h"


#include "nextion_object_func.h"

#include "esp_log.h"

static const char * TAG = "[Nex Crop]";

nextion_crop_img_t *nextion_crop_img_init(const nextion_display_t *display, nextion_descriptor_t *descriptor)
{   
    nextion_crop_img_t *img = calloc(1, sizeof(nextion_crop_img_t));

    if(!img)
    {
        ESP_LOGE(TAG, "Can't allocate");
        return NULL;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(nextion_object_fill(&img->parent, display, descriptor, true));
    img->set_picc = nextion_object_set_picc;

    return img;
}