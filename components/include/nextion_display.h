#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_event.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif
    ESP_EVENT_DECLARE_BASE(NEXTION_EVENT);

    typedef enum
    {
        NEXTION_EVENT_START_OR_RESET = 0x00,
        NEXTION_EVENT_SERIAL_BUFFER_OVERFLOW = 0x24,
        NEXTION_EVENT_TOUCH = 0x65,
        NEXTION_EVENT_CURRENT_PAGE_NUMBER = 0x66,
        NEXTION_EVENT_TOUCH_COORDINATE_AWAKE = 0x67,
        NEXTION_EVENT_TOUCH_COORDINATE_SLEEP = 0x68,
        NEXTION_EVENT_AUTO_ENTERED_SLEEP_MODE = 0x86,
        NEXTION_EVENT_AUTO_WAKE_FROM_SLEEP = 0x87,
        NEXTION_EVENT_DISPLAY_READY = 0x88,
    } nextion_event_t;

    typedef enum
    {
        NEXTION_TOUCH_EVENT_RELEASE,
        NEXTION_TOUCH_EVENT_PRESS
    } nextion_touch_event_type_t;

    typedef enum
    {
        NEXTION_INVALID_INSTRUCTION = 0x00,
        NEXTION_SUCCESFULL_INSTRUCTION = 0x01,
        NEXTION_INVALID_COMPONENT_ID = 0x02,
        NEXTION_INVALID_PAGE_ID = 0x03,
        NEXTION_INVALID_PICTURE_ID = 0x04,
        NEXTION_INVALID_FONT_ID = 0x05,
        NEXTION_INVALID_FILE_OPERATION = 0x06,
        NEXTION_INVALID_CRC = 0x09,
        NEXTION_INVALID_BAUDRATE_SETTING = 0x11,
        NEXTION_INVALID_WAVEFORM_ID_OR_CHANNEL = 0x12,
        NEXTION_INVALID_VARIABLE_NAME_OR_ATTRIBUTE = 0x1A,
        NEXTION_INVALID_VARIABLE_OPERATION = 0x1B,
        NEXTION_ASSIGNEMENT_FAILED_TO_ASSIGN = 0x1C,
        NEXTION_EEPROM_OPERATION_FAILED = 0x1D,
        NEXTION_INVALID_QUANTITY_OF_PARAMETERS = 0x1E,
        NEXTION_IO_OPERATION_FAILED = 0x1F,
        NEXTION_ESCAPE_CHARACTER_INVALID = 0x20,
        NEXTION_VARIABLE_NAME_TOO_LONG = 0x23,
        NEXTION_STRING_DATA_ENCLOSED = 0x070,
        NEXTION_NUMERIC_DATA_ENCLOSED = 0x71,
        NEXTION_TIMEOUT = 0xFD,
        NEXTION_MEM_ERR = 0xFE,
        NEXTION_INVALID_ARGS = 0xFF
    } nextion_err_t;

    typedef struct
    {
        uint8_t dp;
        uint8_t dims;
        uint16_t ussp;
        uint16_t thsp;
        uint8_t thup;
        uint8_t sendxy;
        uint8_t bkcmd;
        uint8_t usup;
    } nextion_system_variables_t;

    typedef struct
    {
        uint8_t page_id;
        uint8_t component_id;
        nextion_touch_event_type_t event_type;
    } nextion_touch_event_data_t;

    typedef struct nextion_display nextion_display_t;


#define NEXTION_SYSTEM_VARIABLES_DEAFULT() { \
    .dp = 0,                                 \
    .dims = 100,                             \
    .ussp = 0,                               \
    .thsp = 30,                              \
    .thup = 0,                               \
    .sendxy = 0,                             \
    .bkcmd = 2,                              \
    .usup = 0                               \
};
    /**
     * @brief Nextion Display Object
     * 
     */
    struct nextion_display
    {
        nextion_err_t (*send_cmd)(nextion_display_t *display, const char *cmd);
        nextion_err_t (*send_cmd_get_value)(nextion_display_t *display, const char *cmd, uint32_t *value);
        nextion_err_t (*send_cmd_get_string)(nextion_display_t *display, const char *cmd, const char *string_ret, size_t *size_ret);
        esp_err_t (*deinit)(nextion_display_t *display);
    };

    /**
     * @brief Configure and initialize Display Object
     * 
     * @param config configuration of Display Object
     *
     * @return nextion_display_t*
     *      - Display Object on success
     *      - NULL on fail
     */
    nextion_display_t *nextion_display_init(nextion_system_variables_t *system_variables);

    /**
     * @brief Register event handler for nextion display event loop
     * 
     * @param display display type object
     * @param event_id nextion_event_t to register
     * @param handler event handler to register
     * @param handler_args arguments for registered handler
     * @return esp_err_t
     *      - ESP_OK on success
     *      - ESP_ERR_NO_MEM on allocating memory for the handler failed
     *      - ESP_ERR_INVALID_ARG on invalid combination of event base and event id
     */
    esp_err_t nextion_register_event_handler(nextion_display_t *display, nextion_event_t event_id, esp_event_handler_t handler, void *handler_args);

    /**
     * @brief Unregister event handler for nextion display event loop
     *
     * @param display display type object
     * @param event_id nextion_event_t to unregister
     * @param handler event handler to unregister
     * @return esp_err_t
     *      - ESP_OK on success
     *      - ESP_ERR_INVALID_ARG on invalid combination of event base and event id
     */
    esp_err_t nextion_unregister_event_handler(nextion_display_t *display, nextion_event_t event_id, esp_event_handler_t handler);

#ifdef __cplusplus
}
#endif