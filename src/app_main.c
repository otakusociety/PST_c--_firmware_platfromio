#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include "esp_bsp.h"
#include "display.h"
#include "lv_port.h"
#include "esp_psram.h"
#include "esp_log.h"

void app_main(void)
{
    printf("PST LVGL Test Start\n");

    esp_err_t ret = esp_psram_init();
    if (ret != ESP_OK) {
        ESP_LOGE("PSRAM", "PSRAM init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI("PSRAM", "PSRAM initialized, free: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
        .rotate = LV_DISP_ROT_90,
    };

    lv_disp_t *disp = bsp_display_start_with_config(&cfg);
    if (!disp) {
        ESP_LOGE("DISPLAY", "Display init FAILED!");
        while(1) vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    bsp_display_backlight_on();
    bsp_display_brightness_set(80);

    ESP_LOGI("DEMO", "Display ready - no demo loaded");

    // Main loop
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}