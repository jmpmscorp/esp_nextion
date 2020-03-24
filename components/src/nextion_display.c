#include "nextion_display.h"

#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_event.h"

/* Nextion Internal Events Headers */
#define NEXTION_EV_SERIAL_BUFFER_OVERFLOW (0x24)
#define NEXTION_EV_TOUCH_EVENT (0x65)
#define NEXTION_EV_CURRENT_PAGE_NUMBER (0x66)
#define NEXTION_EV_TOUCH_COORDINATE_AWAKE (0x67)
#define NEXTION_EV_TOUCH_COORDINATE_SLEEP (0x68)
#define NEXTION_EV_AUTO_ENTERED_SLEEP_MODE (0x86)
#define NEXTION_EV_AUTO_WAKE_FROM_SLEEP (0x87)
#define NEXTION_EV_READY (0x88)
#define NEXTION_EV_START_MICRO_SD_UPGRADE (0x89)
#define NEXTION_EV_TRANSPARENT_DATA_FINISHED (0xFD)
#define NEXTION_EV_TRANSPARENT_DATA_READY (0xFE)

/* UART definitions */
#define NEXTION_UART_NUM CONFIG_NEXTION_UART_NUM

#ifdef CONFIG_NEXTION_CONFIGURE_UART_THRESHOLD
#define NEXTION_UART_TX_GPIO UART_PIN_NO_CHANGE
#define NEXTION_UART_RX_GPIO UART_PIN_NO_CHANGE
#elif defined(CONFIG_NEXTION_UART0_DEFAULT)
#define NEXTION_UART_TX_GPIO GPIO_NUM_1
#define NEXTION_UART_RX_GPIO GPIO_NUM_3
#elif defined(CONFIG_NEXTION_UART1_DEFAULT)
#define NEXTION_UART_TX_GPIO GPIO_NUM_10
#define NEXTION_UART_RX_GPIO GPIO_NUM_9
#elif defined(CONFIG_NEXTION_UART2_DEFAULT)
#define NEXTION_UART_TX_GPIO GPIO_NUM_17
#define NEXTION_UART_RX_GPIO GPIO_NUM_16
#else // CONFIG_NEXTION_UART_CUSTOM
#define NEXTION_UART_TX_GPIO CONFIG_NEXTION_UART_TX_PIN
#define NEXTION_UART_RX_GPIO CONFIG_NEXTION_UART_RX_PIN
#endif

/* Buffer definitions */
#define NEXTION_MSG_BUFFER_SIZE CONFIG_NEXTION_MAX_MSG_LENGTH
/* UART buffers should allocate at least 2 msg */
#define NEXTION_UART_BUFFER_SIZE (NEXTION_MSG_BUFFER_SIZE * 2 > 128 ? NEXTION_MSG_BUFFER_SIZE * 2 + 1 : 129)

/* Queues definitions */
#define NEXTION_UART_QUEUE_SIZE (20)
#define NEXTION_EVENT_QUEUE_SIZE (12)

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

ESP_EVENT_DEFINE_BASE(NEXTION_EVENT);

typedef struct
{
    nextion_err_t *err;
    void *ret_value; /*!< Number or string return by display */
    size_t *size;    /*!< Only usefull when ret_value is string */
} nextion_msg_decode_resources_t;

typedef struct
{
    uart_port_t uart_port;                            /*!< UART port */
    uint8_t *msg_buffer;                              /*!< Internal buffer to store messages from Nextion display */
    nextion_display_t parent;                         /*!< Parent class */
    nextion_system_variables_t system_variables;      /*!< Nextion System Variables */
    nextion_touch_event_data_t *touch_event_data;     /*!< Touch Event associated data */
    nextion_msg_decode_resources_t *decode_resources; /*!< Resources use when any task is waiting response from display */
    QueueHandle_t uart_event_queue;                   /*!< UART event queue handle */
    esp_event_loop_handle_t event_loop_handle;        /*!< Event loop handle */
    TaskHandle_t uart_event_task_handle;              /*!< UART event task handle */
    SemaphoreHandle_t process_sem;                    /*!< Semaphore used for indicating processing status */
    SemaphoreHandle_t access_mutex;                   /*!< Mutex to control unique access to shared display resources */
} esp_nextion_display_t;

/**
 * @brief Process messages receive from display through associated UART and emit event if necessary
 * 
 * @param display 
 */
static void nextion_msg_handler(esp_nextion_display_t *display)
{
    if (!display)
        return;

    switch (display->msg_buffer[0])
    {
        // case 0x00 or 0x00 0x00 0x00
    case NEXTION_INVALID_INSTRUCTION:

        if (display->msg_buffer[1] == 0x00 && display->msg_buffer[2] == 0x00)
        {
            esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_START_OR_RESET, NULL, 0, pdMS_TO_TICKS(100));
        }
        else if (display->decode_resources && display->decode_resources->err)
        {
            *display->decode_resources->err = display->msg_buffer[0];
            xSemaphoreGive(display->process_sem);
        }
        break;

    case NEXTION_SUCCESFULL_INSTRUCTION:
    case NEXTION_INVALID_COMPONENT_ID:
    case NEXTION_INVALID_PAGE_ID:
    case NEXTION_INVALID_PICTURE_ID:
    case NEXTION_INVALID_FONT_ID:
    case NEXTION_INVALID_FILE_OPERATION:
    case NEXTION_INVALID_CRC:
    case NEXTION_INVALID_BAUDRATE_SETTING:
    case NEXTION_INVALID_WAVEFORM_ID_OR_CHANNEL:
    case NEXTION_INVALID_VARIABLE_NAME_OR_ATTRIBUTE:
    case NEXTION_INVALID_VARIABLE_OPERATION:
    case NEXTION_ASSIGNEMENT_FAILED_TO_ASSIGN:
    case NEXTION_EEPROM_OPERATION_FAILED:
    case NEXTION_INVALID_QUANTITY_OF_PARAMETERS:
    case NEXTION_IO_OPERATION_FAILED:
    case NEXTION_ESCAPE_CHARACTER_INVALID:
    case NEXTION_VARIABLE_NAME_TOO_LONG:
        if (display->decode_resources && display->decode_resources->err)
        {
            *display->decode_resources->err = display->msg_buffer[0];
            xSemaphoreGive(display->process_sem);
        }
        break;

    case NEXTION_EV_SERIAL_BUFFER_OVERFLOW:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_SERIAL_BUFFER_OVERFLOW, NULL, 0, pdMS_TO_TICKS(500));
        break;

    case NEXTION_EV_TOUCH_EVENT:
        // Format is: 0x65 0x00 0x01 0x01 0xFF 0xFF 0xFF where:
        // Bit 1: Page number       Bit 2: Component ID         Bit 3: Event (0x01 Press, 0x00 Release)
        display->touch_event_data->page_id = display->msg_buffer[1];
        display->touch_event_data->component_id = display->msg_buffer[2];
        display->touch_event_data->event_type = display->msg_buffer[3];
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_TOUCH, display->touch_event_data, sizeof(nextion_touch_event_data_t), pdMS_TO_TICKS(100));
        break;

    case NEXTION_EV_CURRENT_PAGE_NUMBER:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_CURRENT_PAGE_NUMBER, &display->msg_buffer[1], sizeof(uint8_t), pdMS_TO_TICKS(100));
        break;

    case NEXTION_EV_TOUCH_COORDINATE_SLEEP:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_TOUCH_COORDINATE_SLEEP, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_EV_TOUCH_COORDINATE_AWAKE:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_TOUCH_COORDINATE_AWAKE, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_EV_AUTO_ENTERED_SLEEP_MODE:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_AUTO_ENTERED_SLEEP_MODE, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_EV_AUTO_WAKE_FROM_SLEEP:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_AUTO_WAKE_FROM_SLEEP, NULL, 0, pdMS_TO_TICKS(100));
        break;

    case NEXTION_EV_READY:
        esp_event_post_to(display->event_loop_handle, NEXTION_EVENT, NEXTION_EVENT_DISPLAY_READY, NULL, 0, pdMS_TO_TICKS(100));

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
        bzero(display->msg_buffer, NEXTION_MSG_BUFFER_SIZE);
        read_len = MIN(pos + 3, NEXTION_MSG_BUFFER_SIZE);
        read_len = uart_read_bytes(display->uart_port, display->msg_buffer, read_len, pdMS_TO_TICKS(500));

        if (read_len > 0)
        {
#if CONFIG_NEXTION_ENABLE_DEBUG_UART_DATA
            for (size_t i = 0; i < read_len; ++i)
            {
                printf("%x ", display->msg_buffer[i]);
            }
            printf("\r\n");
#endif
            nextion_msg_handler(display);
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
        if (xQueueReceive(display->uart_event_queue, &event, portMAX_DELAY))
        {
            switch (event.type)
            {
            case UART_DATA:
                break;
            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "HW FIFO Overflow");
                uart_flush_input(display->uart_port);
                xQueueReset(display->uart_event_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "Ring Buffer Full");
                uart_flush_input(display->uart_port);
                xQueueReset(display->uart_event_queue);
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
        // esp_event_loop_run(display->event_loop_handle, pdMS_TO_TICKS(50));
    }
    vTaskDelete(NULL);
}

/**
 * @brief Wait response from display to any command send
 * 
 * @param display[in]   display object
 * @param param1[out]   string when 0x70 is required, number when 0x71
 * @param param2[out]   size of string when 0x70
 * @return nextion_err_t 
 */
static nextion_err_t esp_nextion_display_wait_response(esp_nextion_display_t *display, void *param1, void *param2)
{
    nextion_err_t err;

    if (xSemaphoreTake(display->access_mutex, pdMS_TO_TICKS(250)))
    {

        display->decode_resources = calloc(1, sizeof(nextion_msg_decode_resources_t));

        if (!display->decode_resources)
        {
            xSemaphoreGive(display->access_mutex);
            return NEXTION_MEM_ERR;
        }

        if (display->system_variables.bkcmd == 0)
        {
            // If bkcmd=0, response are disabled. Return always succesfull
            err = NEXTION_SUCCESFULL_INSTRUCTION;
        }
        else
        {
            display->decode_resources->err = &err;
            display->decode_resources->ret_value = param1;
            display->decode_resources->size = param2;

            // If no semaphore take, response isn't ready
            if (xSemaphoreTake(display->process_sem, pdMS_TO_TICKS(250)) == pdFALSE)
            {
                // When bkcmd = 2, display only returns message if there is an error, so if we don't get return, we suppose it's ok
                if (display->system_variables.bkcmd == 2)
                {
                    err = NEXTION_SUCCESFULL_INSTRUCTION;
                }
                else // bkcmd = 1 | bkcmd = 3
                {
                    err = NEXTION_TIMEOUT;
                }
            }
            else // Response is ready
            {
                err = *display->decode_resources->err;
            }
        }

        free(display->decode_resources);
        display->decode_resources = NULL;

        xSemaphoreGive(display->access_mutex);
    }

    return err;
}

/**
 * @brief Send command to display
 * 
 * @param display display object
 * @param cmd  command to send
 * @return nextion_err_t 
 *      - NEXTION_ERR_T value on transfer success
 *      - NEXTION_INVALID_ARGS if any param is null
 *      - NEXTION_MEM_ERR if decode resources allocation fail
 *      - NEXTION_TIMEOUT
 */
static nextion_err_t esp_nextion_display_send_cmd(const nextion_display_t *display, const char *cmd)
{
    NEXTION_CHECK(cmd, "CMD is null", err_args);
    NEXTION_CHECK(display, "Display is NULL", err_args);

    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);

    uart_write_bytes(esp_nextion_display->uart_port, cmd, strlen(cmd));
    uart_write_bytes(esp_nextion_display->uart_port, "\xFF\xFF\xFF", 3);

    return esp_nextion_display_wait_response(esp_nextion_display, NULL, NULL);
err_args:
    return NEXTION_INVALID_ARGS;
}

/**
 * @brief Used when you want to recover number from any object. Display return value 0x71
 * 
 * @param display[in]   display object
 * @param cmd[in]       cmd to send
 * @param number[out]   number returned
 * @return nextion_err_t
 *      - NEXTION_ERR_T value on transfer success
 *      - NEXTION_INVALID_ARGS  if any param is NULL
 *      - NEXTION_MEM_ERR if error allocation decode resources
 *      - Header value returned by display
 */
static nextion_err_t esp_nextion_display_send_cmd_get_number(const nextion_display_t *display, const char *cmd, int32_t *number)
{
    NEXTION_CHECK(cmd, "CMD is null", err_args);
    NEXTION_CHECK(display, "Display is NULL", err_args);
    NEXTION_CHECK(number, "Number is null", err_args);

    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);

    uart_write_bytes(esp_nextion_display->uart_port, cmd, strlen(cmd));
    uart_write_bytes(esp_nextion_display->uart_port, "\xFF\xFF\xFF", 3);

    return esp_nextion_display_wait_response(esp_nextion_display, number, NULL);
err_args:
    return NEXTION_INVALID_ARGS;
}

/**
 * @brief Used when you want to recover string from any object. Display return value 0x70
 * 
 * @param display[in]   display object
 * @param cmd[in]       cmd to send
 * @param string_ret[out]   string returned    
 * @param size_ret[out]     size of string return 
 * @return nextion_err_t
 *      - NEXTION_ERR_T value on transfer success
 *      - NEXTION_INVALID_ARGS  if any param is NULL
 *      - NEXTION_MEM_ERR if error allocation decode resources
 *      - NEXTION_TIMEOUT
 */
static nextion_err_t esp_nextion_display_send_cmd_get_string(const nextion_display_t * display, const char *cmd, const char *string_ret, size_t *size_ret)
{
    NEXTION_CHECK(cmd, "CMD is null", err_args);
    NEXTION_CHECK(display, "Display is NULL", err_args);
    NEXTION_CHECK(string_ret, "String is null", err_args);
    NEXTION_CHECK(size_ret, "Size is NULL", err_args);

    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);

    uart_write_bytes(esp_nextion_display->uart_port, cmd, strlen(cmd));
    uart_write_bytes(esp_nextion_display->uart_port, "\xFF\xFF\xFF", 3);

    return esp_nextion_display_wait_response(esp_nextion_display, string_ret, size_ret);
err_args:
    return NEXTION_INVALID_ARGS;
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
    free(esp_nextion_display->msg_buffer);
    free(esp_nextion_display);

    return ESP_OK;
}

/**
 * @brief Initial configuration for system variables
 * 
 * @param display 
 * @param system_variables 
 */
static void esp_nextion_config_system_variables(esp_nextion_display_t *display, nextion_system_variables_t *system_variables)
{
    nextion_system_variables_t sv = NEXTION_SYSTEM_VARIABLES_DEAFULT();

    if (system_variables)
    {
        if (system_variables->bkcmd != sv.bkcmd)
        {
            sv.bkcmd = system_variables->bkcmd;
        }

        if (system_variables->dp != sv.dp)
        {
            sv.dp = system_variables->dp;
        }

        if (system_variables->dims != sv.dims)
        {
            sv.dims = system_variables->dims;
        }

        if (system_variables->sendxy != sv.sendxy)
        {
            sv.sendxy = system_variables->sendxy;
        }

        if (system_variables->thsp != sv.thsp)
        {
            sv.thsp = system_variables->thsp;
        }

        if (system_variables->thup != sv.thup)
        {
            sv.thup = system_variables->thup;
        }

        if (system_variables->ussp != sv.ussp)
        {
            sv.ussp = system_variables->ussp;
        }

        if (system_variables->usup != sv.usup)
        {
            sv.usup = system_variables->usup;
        }
    }

    display->system_variables = sv;
}

nextion_display_t *nextion_display_init(nextion_system_variables_t *system_variables)
{
    esp_err_t result;
    esp_nextion_display_t *display = calloc(1, sizeof(esp_nextion_display_t));
    NEXTION_CHECK(display, "Calloc Nextion Display failed", err_mem);
    display->msg_buffer = calloc(1, NEXTION_MSG_BUFFER_SIZE);
    NEXTION_CHECK(display->msg_buffer, "Calloc Nextion RX buffer failed", err_buffer);
    display->touch_event_data = calloc(1, sizeof(nextion_touch_event_data_t));
    NEXTION_CHECK(display->touch_event_data, "Calloc Nextion Touch Event data failed", err_touch_event_data);

    /* Initialize object and bind methods */
    display->uart_port = NEXTION_UART_NUM;
    esp_nextion_config_system_variables(display, system_variables);
    display->parent.send_cmd = esp_nextion_display_send_cmd;
    display->parent.deinit = esp_nextion_display_deinit;

    /* Config UART */
    uart_config_t uart_config = {
        .baud_rate = CONFIG_NEXTION_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
#if CONFIG_PM_ENABLE
        .source_clk = UART_SCLK_REF_TICK
#else
        .source_clk = UART_SCLK_APB
#endif
    };

    NEXTION_CHECK(uart_param_config(display->uart_port, &uart_config) == ESP_OK, "config uart parameter failed", err_uart_config);

    result = uart_driver_install(display->uart_port, NEXTION_UART_BUFFER_SIZE, NEXTION_UART_BUFFER_SIZE,
                                 NEXTION_UART_QUEUE_SIZE, &display->uart_event_queue, 0);

    NEXTION_CHECK(result == ESP_OK, "Uart Port driver install failed", err_uart_config);

    result = uart_set_pin(display->uart_port, NEXTION_UART_TX_GPIO, NEXTION_UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    NEXTION_CHECK(result == ESP_OK, "Uart GPIO config failed", err_uart_config);

#ifdef CONFIG_NEXTION_CONFIGURE_UART_THRESHOLD
#if NEXTION_UART_NUM == 0
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_U0RXD);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD_U0TXD);
#elif NEXTION_UART_NUM == 1
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U, FUNC_SD_DATA2_U1RXD);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_SD_DATA3_U1TXD);
#endif

    uart_set_wakeup_threshold(NEXTION_UART_NUM, CONFIG_NEXTION_UART_THRESHOLD);
#endif

    result = uart_enable_pattern_det_baud_intr(display->uart_port, '\xFF', 3, 9, 0, 0);
    /* Set pattern queue size */
    result |= uart_pattern_queue_reset(display->uart_port, NEXTION_UART_QUEUE_SIZE);
    NEXTION_CHECK(result == ESP_OK, "Uart pattern config failed", err_uart_pattern);

    /* Create Event loop */
    esp_event_loop_args_t loop_args = {
        .queue_size = NEXTION_EVENT_QUEUE_SIZE,
        .task_name = "Nextion EV Task",
        .task_stack_size = configMINIMAL_STACK_SIZE * 3,
    };

    NEXTION_CHECK(esp_event_loop_create(&loop_args, &display->event_loop_handle) == ESP_OK, "create event loop failed", err_eloop);

    /* Create semaphore and mutex*/
    display->process_sem = xSemaphoreCreateBinary();
    NEXTION_CHECK(display->process_sem, "Create process semaphore failed", err_sem);
    display->access_mutex = xSemaphoreCreateMutex();
    NEXTION_CHECK(display->access_mutex, "Create mutext failed", err_mux);
    /* Create UART Event task */
    BaseType_t ret = xTaskCreate(uart_event_task,                   //Task Entry
                                 "uart_event",                      //Task Name
                                 2048,                              //Task Stack Size(Bytes)
                                 display,                           //Task Parameter
                                 5,                                 //Task Priority
                                 &(display->uart_event_task_handle) //Task Handler
    );
    NEXTION_CHECK(ret == pdTRUE, "create uart event task failed", err_tsk_create);

    /* Ensure Uart and Nextion are syncronized sending 0xFF 0xFF 0xFF to clean possible garbage on transmission*/
    display->parent.send_cmd(&display->parent, "");

    return &(display->parent);

err_tsk_create:
    vSemaphoreDelete(display->access_mutex);
err_mux:
    vSemaphoreDelete(display->process_sem);
err_sem:
    esp_event_loop_delete(display->event_loop_handle);
err_eloop:
    uart_disable_pattern_det_intr(display->uart_port);

err_uart_pattern:
    uart_driver_delete(display->uart_port);
err_uart_config:
err_touch_event_data:
    free(display->msg_buffer);
err_buffer:
    free(display);

err_mem:
    return NULL;
}

esp_err_t nextion_register_event_handler(const nextion_display_t *display, nextion_event_t event_id, esp_event_handler_t handler, void *handler_args)
{
    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);
    return esp_event_handler_register_with(esp_nextion_display->event_loop_handle, NEXTION_EVENT, event_id, handler, handler_args);
}

esp_err_t nextion_unregister_event_handler(const nextion_display_t *display, nextion_event_t event_id, esp_event_handler_t handler)
{
    esp_nextion_display_t *esp_nextion_display = __containerof(display, esp_nextion_display_t, parent);
    return esp_event_handler_unregister_with(esp_nextion_display->event_loop_handle, NEXTION_EVENT, event_id, handler);
}