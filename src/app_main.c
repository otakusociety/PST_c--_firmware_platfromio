#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include "esp_bsp.h"
#include "esp_psram.h"
#include "esp_log.h"
#include "pincfg.h"
#include "display.h"
#include "esp_lvgl_port.h"
#include "pst_file_browser.h"
#include "pst_keyboard.h"

static const char *TAG = "EXPLORER_TEST";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing System...");

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .rotate = LV_DISPLAY_ROTATION_90,
    };

    // Start display + touch
    lv_disp_t *disp = bsp_display_start_with_config(&cfg);
    if (!disp)
    {
        ESP_LOGE(TAG, "Failed to start display");
        return;
    }

    bsp_display_backlight_on();

    // RED SCREEN
    if (bsp_display_lock(1000))
    {
        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_make(255, 0, 0), 0); // ✅ FIXED!
        bsp_display_unlock();
    }

    ESP_LOGI(TAG, "Entering LVGL loop...");

    // 🔥 CRITICAL: LVGL NEEDS THIS EVERY 10ms!
    while (1)
    {
        // Run LVGL timers
        uint32_t ms_to_next = lv_timer_handler();

        // Use the returned time to sleep efficiently
        if (ms_to_next == 0)
            ms_to_next = 1;
        vTaskDelay(pdMS_TO_TICKS(ms_to_next));
    }
}
