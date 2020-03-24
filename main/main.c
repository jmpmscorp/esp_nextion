#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_pm.h"

#include "nextion_display.h"
#include "nextion_dual_state_button.h"
#include "nextion_crop_img.h"
#include "nextion_button.h"
#include "nextion_xfloat.h"
#include "nextion_number.h"
#include "nextion_text.h"
#include "nextion_variable.h"

static const char *TAG = "[MAIN EXAMPLE]";

const nextion_display_t *display;
const nextion_button_t *btn0;
const nextion_xfloat_t *xfloat0;
const nextion_number_t *n0;
const nextion_crop_img_t *q0;

static void nextion_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Nextion Event %x", event_id);

    if (event_id == NEXTION_EVENT_DISPLAY_READY)
    {
        ESP_LOGI(TAG, "Display Ready");
        ESP_LOGI(TAG, "Set sys0 ret: 0x%2x", display->send_cmd(display, "sys0=2"));
    }
    else if (event_id == NEXTION_EVENT_CURRENT_PAGE_NUMBER)
    {
        uint8_t page = *(uint8_t *)event_data;
        ESP_LOGI(TAG, "Page Event");
        if (page == 1)
        {
            ESP_LOGI(TAG, "Set BTN TXT ret: 0x%2x", btn0->set_text(btn0, "VINA"));
        }
    }
    else if (event_id == NEXTION_EVENT_TOUCH)
    {
        static int8_t rssi = -120;
        static uint16_t pic_number = 2;
        nextion_touch_event_data_t *data = (nextion_touch_event_data_t *)event_data;
        ESP_LOGI(TAG, "Touch Event: Page->%d | Component ID->%d | Event->%d", data->page_id, data->component_id, data->event_type);

        if (data->page_id == btn0->parent.descriptor.page_id && data->component_id == btn0->parent.descriptor.component_id)
        {
            rssi += 5;
            if(pic_number == 2)
            {
                pic_number = 3;
            }
            else
            {
                pic_number = 2;
            }
            
            n0->set_value(n0, rssi);
            q0->set_picc(q0, pic_number);
        }
    }
    else if (event_id == NEXTION_EVENT_AUTO_ENTERED_SLEEP_MODE)
    {
        
    }
    else if (event_id == NEXTION_EVENT_AUTO_WAKE_FROM_SLEEP)
    {
    }
}

void app_main(void)
{    
    display = (const nextion_display_t *)nextion_display_init(NULL);
    nextion_register_event_handler(display, ESP_EVENT_ANY_ID, nextion_event_handler, (void *)display);

    nextion_descriptor_t descriptor = {
        .page_id = 1,
        .component_id = 1,
        .name = "b0"};

    btn0 = nextion_button_init(display, &descriptor);

    descriptor.component_id = 4;
    descriptor.name = "x0";

    xfloat0 = nextion_xfloat_init(display, &descriptor);

    descriptor.component_id = 5;
    descriptor.name = "q0";

    q0 = nextion_crop_img_init(display, &descriptor);

    descriptor.component_id = 3;
    descriptor.name = "n0";

    n0 = nextion_number_init(display, &descriptor);

    display->send_cmd(display, "sleep=0");
    display->send_cmd(display, "rest");
}