#include "esp_nextion_display.h"

#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_event.h"

#define NEXTION_RET_INVALID_INSTRUCTION (0x00)
#define NEXTION_RET_SUCCESFULL_INSTRUCTION (0x01)
#define NEXTION_RET_INVALID_COMPONENT_ID (0x02)
#define NEXTION_RET_INVALID_PAGE_ID (0x03)
#define NEXTION_RET_INVALID_PICTURE_ID (0x04)
#define NEXTION_RET_INVALID_FONT_ID (0x05)
#define NEXTION_RET_INVALID_FILE_OPERATION (0x06)
#define NEXTION_RET_INVALID_CRC (0x09)
#define NEXTION_RET_INVALID_BAUDRATE_SETTING (0x11)
#define NEXTION_RET_INVALID_WAVEFORM_ID_OR_CHANNEL (0x12)
#define NEXTION_RET_INVALID_VARIABLE_NAME_OR_ATTRIBUTE (0x1A)
#define NEXTION_RET_INVALID_VARIABLE_OPERATION (0x1B)
#define NEXTION_RET_ASSIGNEMENT_FAILED_TO_ASSIGN (0x1C)
#define NEXTION_RET_EEPROM_OPERATION_FAILED (0x1D)
#define NEXTION_RET_INVALID_QUANTITY_OF_PARAMETERS (0x1E)
#define NEXTION_RET_IO_OPERATION_FAILED (0x1F)
#define NEXTION_RET_ESCAPE_CHARACTER_INVALID (0x20)
#define NEXTION_RET_VARIABLE_NAME_TOO_LONG (0x23)
#define NEXTION_RET_SERIAL_BUFFER_OVERFLOW (0x24)
#define NEXTION_RET_TOUCH_EVENT (0x65)
#define NEXTION_RET_CURRENT_PAGE_NUMBER (0x66)
#define NEXTION_RET_TOUCH_COORDINATE_AWAKE (0x67)
#define NEXTION_RET_TOUCH_COORDINATE_SLEEP (0x68)
#define NEXTION_RET_STRING_DATA_ENCLOSED (0x70)
#define NEXTION_RET_NUMERIC_DATA_ENCLOSED (0x71)
#define NEXTION_RET_AUTO_ENTERED_SLEEP_MODE (0x86)
#define NEXTION_RET_AUTO_WAKE_FROM_SLEEP (0x87)
#define NEXTION_RET_READY (0x88)
#define NEXTION_RET_START_MICRO_SD_UPGRADE (0x89)
#define NEXTION_RET_TRANSPARENT_DATA_FINISHED (0xFD)
#define NEXTION_RET_TRANSPARENT_DATA_READY (0xFE)

#ifdef CONFIG_NEXTION_UART_BUFFER_SIZE
#define ESP_NEXTION_UART_RX_BUFFER_SIZE CONFIG_NEXTION_UART_BUFFER_SIZE
#else
#define ESP_NEXTION_UART_BUFFER_SIZE (65)
#endif

#define ESP_NEXTION_EVENT_QUEUE_SIZE (16)

static const char *TAG = "[Nextion Display]";

#define NEXTION_CHECK(a, str, goto_tag, ...)                                      \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                        \
        }                                                                         \
    } while (0)

ESP_EVENT_DEFINE_BASE(ESP_NEXTION_EVENT);

typedef struct
{
    uart_port_t uart_port;                        /*!< UART port */
    uint8_t *buffer;                              /*!< Internal buffer to store response lines/data from DCE */
    nextion_display_t parent;                     /*!< Parent class */
    nextion_touch_event_data_t *touch_event_data; /*!< Touch Event associated data */
    QueueHandle_t event_queue;                    /*!< UART event queue handle */
    esp_event_loop_handle_t event_loop_handle;    /*!< Event loop handle */
    TaskHandle_t uart_event_task_handle;          /*!< UART event task handle */
    SemaphoreHandle_t process_sem;                /*!< Semaphore used for indicating processing status */
} esp_nextion_display_t;

/**
 * @brief Process commands receive from display through associated UART and emit event if necessary
 * 
 * @param display 
 */
static void process_display_response(esp_nextion_display_t *display)
{
    if (!display)
        return;

    switch (display->buffer[0])
    {
        // case 0x00 or 0x00 0x00 0x00
    case NEXTION_RET_INVALID_INSTRUCTION:

        if (display->buffer[1] == 0x00 && display->buffer[2] == 0x00)
        {
            esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_START_OR_RESET, NULL, 0, pdMS_TO_TICKS(100));
        }
        break;

    case NEXTION_RET_SERIAL_BUFFER_OVERFLOW:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_SERIAL_BUFFER_OVERFLOW, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_RET_TOUCH_EVENT:
        // Format is: 0x65 0x00 0x01 0x01 0xFF 0xFF 0xFF where:
        // Bit 1: Page number       Bit 2: Component ID         Bit 3: Event (0x01 Press, 0x00 Release)
        display->touch_event_data->page_id = display->buffer[1];
        display->touch_event_data->component_id = display->buffer[2];
        display->touch_event_data->event_type = display->buffer[3];
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_TOUCH, display->touch_event_data, sizeof(nextion_touch_event_data_t), pdMS_TO_TICKS(100));
        break;

    case NEXTION_RET_CURRENT_PAGE_NUMBER:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_CURRENT_PAGE_NUMBER, &display->buffer[1], sizeof(uint8_t), pdMS_TO_TICKS(100));
        break;

    case NEXTION_RET_TOUCH_COORDINATE_SLEEP:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_TOUCH_COORDINATE_SLEEP, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_RET_TOUCH_COORDINATE_AWAKE:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_TOUCH_COORDINATE_AWAKE, NULL, 0, pdMS_TO_TICKS(100));

    case NEXTION_RET_AUTO_ENTERED_SLEEP_MODE:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_AUTO_ENTERED_SLEEP_MODE, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_RET_AUTO_WAKE_FROM_SLEEP:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_AUTO_WAKE_FROM_SLEEP, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_RET_READY:
        esp_event_post_to(display->event_loop_handle, ESP_NEXTION_EVENT, ESP_NEXTION_EVENT_DISPLAY_READY, NULL, 0, pdMS_TO_TICKS(100));

    default:
        break;
    }
}

/**
 * @brief Handle when a pattern has been detected by UART
 *
 * @param esp_dte ESP32 Modem DTE object
 */
static void esp_handle_uart_pattern(esp_nextion_display_t *display)
{
    int pos = uart_pattern_pop_pos(display->uart_port);
    int read_len = 0;

    if (pos != -1)
    {
        bzero(display->buffer, ESP_NEXTION_UART_BUFFER_SIZE);
        read_len = pos + 3;
        read_len = uart_read_bytes(display->uart_port, display->buffer, read_len, pdMS_TO_TICKS(500));

        if (read_len > 0)
        {
            // ESP_LOGI(TAG, "Receive: %d", read_len);
            for (size_t i = 0; i < read_len; ++i)
            {
                printf("%x ", display->buffer[i]);
            }
            printf("\r\n");
            process_display_response(display);
        }
        else
        {
            ESP_LOGE(TAG, "uart read bytes failed");
        }
    }
    else
    {
        ESP_LOGW(TAG, "Pattern Queue Size too small");
        uart_flush(display->uart_port);
    }
}

/**
 * @brief Handle when new data received by UART
 *
 * @param esp_dte ESP32 Modem DTE object
 */
static void esp_handle_uart_data(esp_nextion_display_t *display)
{
    ESP_LOGI(TAG, "Received uart data");
    size_t length = 0;
    uart_get_buffered_data_len(display->uart_port, &length);
    length = MIN(ESP_NEXTION_UART_BUFFER_SIZE, length);

    length = uart_read_bytes(display->uart_port, display->buffer, length, portMAX_DELAY);
}

/**
 * @brief UART Event Task Entry
 *
 * @param param task parameter
 */
static void uart_event_task(void *arg)
{
    esp_nextion_display_t *display = (esp_nextion_display_t *)arg;
    uart_event_t event;
    ESP_LOGI(TAG, "Uart event task started");

    while (true)
    {
        if (xQueueReceive(display->event_queue, &event, portMAX_DELAY))
        {
            switch (event.type)
            {
            case UART_DATA:
                esp_handle_uart_data(display);
                break;
            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "HW FIFO Overflow");
                uart_flush_input(display->uart_port);
                xQueueReset(display->event_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "Ring Buffer Full");
                uart_flush_input(display->uart_port);
                xQueueReset(display->event_queue);
                break;
            case UART_BREAK:
                ESP_LOGW(TAG, "Rx Break");
                break;
            case UART_PARITY_ERR:
                ESP_LOGE(TAG, "Parity Error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGE(TAG, "Frame Error");
                break;
            case UART_PATTERN_DET:
                esp_handle_uart_pattern(display);
                break;
            default:
                ESP_LOGW(TAG, "unknown uart event type: %d", event.type);
                break;
            }
        }
        /* Drive the event loop */
        esp_event_loop_run(display->event_loop_handle, pdMS_TO_TICKS(50));
    }
    vTaskDelete(NULL);
}

/**
 * @brief Send command to display
 * 
 * @param display display object
 * @param cmd  command to send
 * @param timeout   maximum timeout to wait response, if needed
 * @return esp_err_t 
 *      - ESP_OK on success
 *      - ESP_FAIL otherwise
 */
static esp_err_t esp_nextion_display_send_cmd(nextion_display_t *display, const char *cmd, uint32_t timeout)
{
    NEXTION_CHECK(cmd, "CMD is null", err);
    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);
    uart_write_bytes(esp_nextion_display->uart_port, cmd, strlen(cmd));
    uart_write_bytes(esp_nextion_display->uart_port, "\xFF\xFF\xFF", 3);
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Free display resources
 * 
 * @param display display object
 * @return esp_err_t 
 *      - ESP_OK
 *      - ESP_FAIL if null display param
 */
static esp_err_t esp_nextion_display_deinit(nextion_display_t *display)
{
    if (!display)
        return ESP_FAIL;

    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);

    if (esp_nextion_display->uart_event_task_handle)
    {
        vTaskDelete(esp_nextion_display->uart_event_task_handle);
    }

    if (esp_nextion_display->process_sem)
    {
        vSemaphoreDelete(esp_nextion_display->process_sem);
    }

    if (esp_nextion_display->event_loop_handle)
    {
        esp_event_loop_delete(esp_nextion_display->event_loop_handle);
    }

    uart_driver_delete(esp_nextion_display->uart_port);

    free(esp_nextion_display->touch_event_data);
    free(esp_nextion_display->buffer);
    free(esp_nextion_display);

    return ESP_OK;
}

nextion_display_t *esp_nextion_display_init(const esp_nextion_display_config_t *config)
{
    esp_err_t result;
    esp_nextion_display_t *display = calloc(1, sizeof(esp_nextion_display_t));
    NEXTION_CHECK(display, "Calloc Nextion Display failed", err_mem);
    display->buffer = calloc(1, ESP_NEXTION_UART_BUFFER_SIZE);
    NEXTION_CHECK(display->buffer, "Calloc Nextion RX buffer failed", err_buffer);
    display->touch_event_data = calloc(1, sizeof(nextion_touch_event_data_t));
    NEXTION_CHECK(display->touch_event_data, "Calloc Nextion Touch Event data failed", err_touch_event_data);

    /* Initialize object and bind methods */
    display->uart_port = config->uart.uart_port;

    display->parent.send_cmd = esp_nextion_display_send_cmd;
    display->parent.deinit = esp_nextion_display_deinit;

    /* Config UART */
    uart_config_t uart_config = {
        .baud_rate = config->uart.baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB};

    result = uart_driver_install(display->uart_port, ESP_NEXTION_UART_BUFFER_SIZE * 2, ESP_NEXTION_UART_BUFFER_SIZE * 2,
                                 config->uart.event_queue_size, &display->event_queue, 0);

    NEXTION_CHECK(result == ESP_OK, "Uart Port driver install failed", err_uart_config);
    NEXTION_CHECK(uart_param_config(display->uart_port, &uart_config) == ESP_OK, "config uart parameter failed", err_uart_config);

    result = uart_set_pin(display->uart_port, config->uart.tx_pin, config->uart.rx_pin,
                          UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    NEXTION_CHECK(result == ESP_OK, "Uart GPIO config failed", err_uart_config);

    if (config->uart.rx_pin == UART_PIN_NO_CHANGE && config->uart.tx_pin == UART_PIN_NO_CHANGE && config->uart.wakeup_threshold >= 3)
    {
        if (config->uart.uart_port == UART_NUM_0 || config->uart.uart_port == UART_NUM_1)
        {
            if (config->uart.uart_port == UART_NUM_0)
            {
                PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_U0RXD);
                PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD_U0TXD);
            }
            else
            {
                PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U, FUNC_SD_DATA2_U1RXD);
                PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_SD_DATA3_U1TXD);
            }

            uart_set_wakeup_threshold(config->uart.uart_port, config->uart.wakeup_threshold);
        }

        else if (config->uart.uart_port == UART_NUM_2)
        {
            ESP_LOGE(TAG, "Ligth sleep wake up source is not available in Uart 2");
        }
    }

    result = uart_enable_pattern_det_baud_intr(display->uart_port, '\xFF', 3, 9, 0, 0);
    /* Set pattern queue size */
    result |= uart_pattern_queue_reset(display->uart_port, config->uart.event_queue_size);
    NEXTION_CHECK(result == ESP_OK, "Uart pattern config failed", err_uart_pattern);

    /* Create Event loop */
    esp_event_loop_args_t loop_args = {
        .queue_size = ESP_NEXTION_EVENT_QUEUE_SIZE,
        .task_name = NULL};
    NEXTION_CHECK(esp_event_loop_create(&loop_args, &display->event_loop_handle) == ESP_OK, "create event loop failed", err_eloop);

    /* Create semaphore */
    display->process_sem = xSemaphoreCreateBinary();
    NEXTION_CHECK(display->process_sem, "Create process semaphore failed", err_sem);

    /* Create UART Event task */
    BaseType_t ret = xTaskCreate(uart_event_task,                   //Task Entry
                                 "uart_event",                      //Task Name
                                 2048,                              //Task Stack Size(Bytes)
                                 display,                           //Task Parameter
                                 5,                                 //Task Priority
                                 &(display->uart_event_task_handle) //Task Handler
    );
    NEXTION_CHECK(ret == pdTRUE, "create uart event task failed", err_tsk_create);
    return &(display->parent);

err_tsk_create:
    vSemaphoreDelete(display->process_sem);
err_sem:
    esp_event_loop_delete(display->event_loop_handle);
err_eloop:
    uart_disable_pattern_det_intr(display->uart_port);

err_uart_pattern:
    uart_driver_delete(display->uart_port);
err_uart_config:
err_touch_event_data:
    free(display->buffer);
err_buffer:
    free(display);

err_mem:
    return NULL;
}

esp_err_t esp_nextion_register_event_handler(nextion_display_t *display, esp_nextion_event_t event_id, esp_event_handler_t handler, void *handler_args)
{
    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);
    return esp_event_handler_register_with(esp_nextion_display->event_loop_handle, ESP_NEXTION_EVENT, event_id, handler, handler_args);
}

esp_err_t esp_nextion_unregister_event_handler(nextion_display_t *display, esp_nextion_event_t event_id, esp_event_handler_t handler)
{
    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);
    return esp_event_handler_unregister_with(esp_nextion_display->event_loop_handle, ESP_NEXTION_EVENT, event_id, handler);
}