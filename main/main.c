#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_system.h"

#include "esp_nextion_display.h"
#include "esp_nextion_object.h"

static const char *TAG = "[EXAMPLE MAIN]";

static void nextion_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Nextion Event %x", event_id);
}

void app_main(void)
{
    esp_nextion_display_config_t config = ESP_NEXTION_DISPLAY_CONFIG_DEFAULT();
    config.uart.tx_pin = 4;
    config.uart.rx_pin = 5;
    nextion_display_t *display = esp_nextion_display_init(&config);
    esp_nextion_register_event_handler(display, ESP_EVENT_ANY_ID, nextion_event_handler, display);

    nextion_descriptor_t descriptor = {
        .page_id = 1,
        .component_id = 21,
        .name = "bt0"};

    nextion_dual_state_button_t *btn = esp_nextion_dual_state_button_init(&descriptor);

    display->send_cmd(display, "sleep=0", 1000);

    btn->set_value(display, btn, 0);
    //display->send_cmd(display, "rest", 1000);
    //display->send_cmd(display, "bt0.val=1", 1000);
}
