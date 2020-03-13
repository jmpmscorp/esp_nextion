#include "esp_nextion_object.h"

#include <string.h>

#include "esp_log.h"

typedef enum
{
    NEXTION_BUTTON_OBJECT,
    NEXTION_DUAL_STATE_BUTTON_OBJECT,
    NEXTION_NUMBER_OBJECT,
    NEXTION_TEXT_OBJECT,
    NEXTION_XFLOAT_OBJECT
} nextion_object_type_t;

typedef struct
{
    nextion_descriptor_t descriptor;
    nextion_object_type_t type;
    bool touchable;
    void *parent;
} esp_nextion_object_t;

#define NEXTION_OBJECT_CHECK(a, tag, str, goto_tag, ...)                          \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(tag, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                        \
        }                                                                         \
    } while (0)

static esp_err_t esp_nextion_object_set_value(nextion_display_t *display, void *object, uint32_t value)
{
    esp_nextion_object_t *obj = __containerof(object, esp_nextion_object_t, parent);
    char *cmd = malloc(40 * sizeof(char));
    bzero(cmd, 40);
    ESP_LOGI("Object", "page%d.%s.val=%d", obj->descriptor.page_id, obj->descriptor.name, value);
    //sprintf(cmd, "page%d.%s.val=%d", obj->descriptor.page_id, obj->descriptor.name, value);
    //return display->send_cmd(display, cmd, 1000);
    return ESP_OK;
}

nextion_dual_state_button_t *esp_nextion_dual_state_button_init(nextion_descriptor_t *descriptor)
{
    const char *tag = "[Nextion Dual Button]";

    NEXTION_OBJECT_CHECK(descriptor, tag, "Descriptor is NULL", err);

    esp_nextion_object_t *obj = calloc(1, sizeof(esp_nextion_object_t));
    NEXTION_OBJECT_CHECK(obj, tag, "Calloc object failed", err);
    nextion_dual_state_button_t * btn = calloc(1,sizeof(nextion_dual_state_button_t));
    NEXTION_OBJECT_CHECK(obj, tag, "Calloc dual button failed", err_alloc_parent);
    
    btn->set_value = esp_nextion_object_set_value;
    obj->descriptor.page_id = descriptor->page_id;
    obj->descriptor.component_id = descriptor->component_id;
    obj->descriptor.name = strdup(descriptor->name);
    obj->parent = btn;
    obj->touchable = false;
    obj->type = NEXTION_DUAL_STATE_BUTTON_OBJECT;

    return btn;
err_alloc_parent:
    free(obj);
err:
    return NULL;
}