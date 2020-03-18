#include "esp_nextion_object.h"

#include <string.h>

#include "esp_log.h"

static const char * TAG = "[Nextion Object]";

#define NEXTION_OBJECT_CHECK(a, str, goto_tag, ...)                          \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                        \
        }                                                                         \
    } while (0)


esp_err_t esp_nextion_object_fill(nextion_object_t *object, nextion_display_t *display, nextion_descriptor_t *descriptor, bool touchable)
{
    NEXTION_OBJECT_CHECK(object, "NUll object passed", err);
    NEXTION_OBJECT_CHECK(display, "NULL display", err);
    NEXTION_OBJECT_CHECK(descriptor, "NULL display", err);


    object->descriptor.page_id = descriptor->page_id;
    object->descriptor.component_id = descriptor->component_id;
    object->descriptor.name = strdup(descriptor->name);
    object->display = display;
    object->touchable = touchable;

    ESP_LOGD("Object", "page%d %d %s", object->descriptor.page_id, object->descriptor.component_id, object->descriptor.name);

    return ESP_OK;
err:
    return ESP_ERR_INVALID_ARG;
}