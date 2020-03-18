#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"

#include "esp_nextion_display.h"
#include "esp_nextion_dual_state_button.h"
#include "esp_nextion_xfloat.h"
#include "esp_nextion_text.h"
#include "esp_nextion_variable.h"

static const char *TAG = "[EXAMPLE MAIN]";

const nextion_dual_state_button_t *dsbtn0;
const nextion_xfloat_t *xfloat0;
const nextion_text_t *t0;
const nextion_variable_t *vtubes;

static void nextion_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Nextion Event %x", event_id);

    if(event_id == ESP_NEXTION_EVENT_TOUCH)
    {
        static int16_t value = -1;
        static uint8_t tubes = 1;
        nextion_touch_event_data_t *data = (nextion_touch_event_data_t *)event_data;
        ESP_LOGI(TAG, "Touch Event: Page->%d | Component ID->%d | Event->%d", data->page_id, data->component_id, data->event_type);
        
        if(data->page_id == dsbtn0->parent.descriptor.page_id && data->component_id == dsbtn0->parent.descriptor.component_id)
        {
            value += value;
            ++tubes;

            if(tubes > 4)
            {
                tubes = 1;
            }

            xfloat0->set_value(xfloat0, value);
        }
    }
}

void app_main(void)
{
    esp_nextion_display_config_t config = ESP_NEXTION_DISPLAY_CONFIG_DEFAULT();
    config.uart.tx_pin = GPIO_NUM_25;
    config.uart.rx_pin = GPIO_NUM_26;

    nextion_display_t *display = esp_nextion_display_init(&config);
    esp_nextion_register_event_handler(display, ESP_EVENT_ANY_ID, nextion_event_handler, display);    

    nextion_descriptor_t descriptor = {
        .page_id = 1,
        .component_id = 21,
        .name = "bt0"
    };

    dsbtn0 = esp_nextion_dual_state_button_init(display, &descriptor);

    descriptor.component_id = 5;
    descriptor.name = "x0";

    xfloat0 = esp_nextion_xfloat_init(display, &descriptor);

    descriptor.component_id = 13;
    descriptor.name = "t0";

    t0 = esp_nextion_text_init(display, &descriptor);

    descriptor.component_id = 13;
    descriptor.name = "vtubes";

    vtubes = esp_nextion_variable_init(display, &descriptor);

    display->send_cmd(display, "sleep=0", 1000);

    t0->set_text(t0, "Vina");
    vtubes->set_number_value(vtubes, 2);
    // display->send_cmd(display, "ref page1", 1000);
    // dsbtn0->set_value(dsbtn0, 1);
    //display->send_cmd(display, "rest", 1000);
    //display->send_cmd(display, "bt0.val=1", 1000);
}
