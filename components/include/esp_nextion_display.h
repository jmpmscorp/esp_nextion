#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_event.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif
    ESP_EVENT_DECLARE_BASE(ESP_NEXTION_EVENT);

    typedef enum
    {
        ESP_NEXTION_EVENT_START_OR_RESET = 0x00,
        ESP_NEXTION_EVENT_SERIAL_BUFFER_OVERFLOW = 0x24,
        ESP_NEXTION_EVENT_TOUCH = 0x65,
        ESP_NEXTION_EVENT_CURRENT_PAGE_NUMBER = 0x66,
        ESP_NEXTION_EVENT_TOUCH_COORDINATE_AWAKE = 0x67,
        ESP_NEXTION_EVENT_TOUCH_COORDINATE_SLEEP = 0x68,
        ESP_NEXTION_EVENT_AUTO_ENTERED_SLEEP_MODE = 0x86,
        ESP_NEXTION_EVENT_AUTO_WAKE_FROM_SLEEP = 0x87,
        ESP_NEXTION_EVENT_DISPLAY_READY = 0x88,
    } esp_nextion_event_t;

    typedef struct
    {
        struct
        {
            uart_port_t uart_port;     /*!< UART port number */
            uint32_t rx_pin;           /*!< UART RX Pin number */
            uint32_t tx_pin;           /*!< UART TX Pin number */
            uint32_t baud_rate;        /*!< UART baud rate */
            uint32_t event_queue_size; /*!< UART event queue size */
        } uart;
    } esp_nextion_display_config_t;

/**
     * @brief ESP Nextion
     *
     */
#define ESP_NEXTION_DISPLAY_CONFIG_DEFAULT() \
    {                                        \
        .uart =                              \
        {                                    \
            .uart_port = UART_NUM_2,         \
            .rx_pin = GPIO_NUM_16,           \
            .tx_pin = GPIO_NUM_17,           \
            .baud_rate = 9600,               \
            .event_queue_size = 6            \
        }                                    \
    }

    typedef struct nextion_display nextion_display_t;

    /**
     * @brief Nextion Display Object
     * 
     */
    struct nextion_display
    {
        esp_err_t (*send_cmd)(nextion_display_t *display, const char *cmd, uint32_t timeout);
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
    nextion_display_t *esp_nextion_display_init(const esp_nextion_display_config_t *config);

    /**
     * @brief Register event handler for nextion display event loop
     * 
     * @param display display type object
     * @param event_id esp_nextion_event_t to register
     * @param handler event handler to register
     * @param handler_args arguments for registered handler
     * @return esp_err_t
     *      - ESP_OK on success
     *      - ESP_ERR_NO_MEM on allocating memory for the handler failed
     *      - ESP_ERR_INVALID_ARG on invalid combination of event base and event id
     */
    esp_err_t esp_nextion_register_event_handler(nextion_display_t *display, esp_nextion_event_t event_id, esp_event_handler_t handler, void *handler_args);

    /**
     * @brief Unregister event handler for nextion display event loop
     *
     * @param display display type object
     * @param event_id esp_nextion_event_t to unregister
     * @param handler event handler to unregister
     * @return esp_err_t
     *      - ESP_OK on success
     *      - ESP_ERR_INVALID_ARG on invalid combination of event base and event id
     */
    esp_err_t esp_nextion_unregister_event_handler(nextion_display_t *display, esp_nextion_event_t event_id, esp_event_handler_t handler);

#ifdef __cplusplus
}
#endif