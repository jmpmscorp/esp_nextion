#include "nextion_object_func.h"

#include <string.h>

#include "esp_log.h"

static const char *TAG = "[Nextion Service]";

#define NEXTION_OBJ_SERVICE_CHECK(a, str, ret, ...)                               \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            return ret;                                                           \
        }                                                                         \
    } while (0)

#define NEXTION_OBJ_SERVICE_DOUBLE_CHECK(a, b, str, ret, ...)                     \
    do                                                                            \
    {                                                                             \
        if (!(a) || !(b))                                                         \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            return ret;                                                           \
        }                                                                         \
    } while (0)

nextion_err_t nextion_object_set_number_value(const void *object, int32_t value)
{
    nextion_object_t *obj = (nextion_object_t *)object;
    NEXTION_OBJ_SERVICE_CHECK(obj, "Object NULL", ESP_ERR_INVALID_ARG);
    char *cmd = malloc(40 * sizeof(char));
    bzero(cmd, 40);
    sprintf(cmd, "%s.val=%d", obj->descriptor.name, value);
    nextion_err_t result = obj->display->send_cmd(obj->display, cmd);
    free(cmd);
    return result;
}

nextion_err_t nextion_object_set_string_value(const void *object, const char *value)
{
    nextion_object_t *obj = (nextion_object_t *)object;
    NEXTION_OBJ_SERVICE_DOUBLE_CHECK(obj, value, "Object or value NULL", ESP_ERR_INVALID_ARG);
    char *cmd = malloc(40 * sizeof(char));
    bzero(cmd, 40);
    sprintf(cmd, "%s.val=\"%s\"", obj->descriptor.name, value);
    nextion_err_t result = obj->display->send_cmd(obj->display, cmd);
    free(cmd);
    return result;
}

nextion_err_t nextion_object_set_text(const void *object, const char *text)
{
    nextion_object_t *obj = (nextion_object_t *)object;
    NEXTION_OBJ_SERVICE_DOUBLE_CHECK(obj, text, "Object or value NULL", ESP_ERR_INVALID_ARG);
    char *cmd = malloc(40 * sizeof(char));
    bzero(cmd, 40);
    sprintf(cmd, "%s.txt=\"%s\"", obj->descriptor.name, text);
    nextion_err_t result = obj->display->send_cmd(obj->display, cmd);
    free(cmd);
    return result;
}

nextion_err_t nextion_object_set_picc(const void *object, uint32_t number)
{
    nextion_object_t *obj = (nextion_object_t *)object;
    NEXTION_OBJ_SERVICE_CHECK(obj, "Object NULL", ESP_ERR_INVALID_ARG);
    char *cmd = malloc(40 * sizeof(char));
    bzero(cmd, 40);
    sprintf(cmd, "%s.picc=%d", obj->descriptor.name, number);
    nextion_err_t result = obj->display->send_cmd(obj->display, cmd);
    free(cmd);
    return result;
}