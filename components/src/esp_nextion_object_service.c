#include "esp_nextion_object_service.h"

#include <string.h>

#include "esp_nextion_display.h"
#include "esp_nextion_object.h"

#include "esp_log.h"

static const char * TAG = "[Nextion Service]";

#define NEXTION_OBJ_SERVICE_CHECK(a, str, ret,...)                                \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            return ret;                                                           \
        }                                                                         \
    } while (0)


esp_err_t esp_nextion_object_set_value(void * object, uint32_t value)
{
    nextion_object_t *obj = (nextion_object_t *)object;
    NEXTION_OBJ_SERVICE_CHECK(object, "Object NULL", ESP_ERR_INVALID_ARG);
    
    char *cmd = malloc(40 * sizeof(char));
    bzero(cmd, 40);
    //ESP_LOGI("Object", "page%d.%s.val = %d", obj->descriptor.page_id, obj->descriptor.name, value);
    sprintf(cmd, "page%d.%s.val=%d", obj->descriptor.page_id, obj->descriptor.name, value);
    return obj->display->send_cmd(obj->display, cmd, 1000);
    return ESP_OK;

}