#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>

#include "esp_bsp.h"
#include "esp_psram.h"
#include "esp_log.h"
#include "pincfg.h"
#include "display.h"

#include "lv_port.h"
#include "pst_file_browser.h"
#include "pst_keyboard.h"
#include "boot_screen.h"

static const char *TAG = "EXPLORER_TEST_WITH_BOOT_SCREEN";

void my_file_picker_callback(const char *full_path)
{
    if (full_path)
    {
        ESP_LOGI(TAG, "Selected: %s", full_path);
    }
}

void app_main(void)
{
    // --- STEP A: DISPLAY INIT ---

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES *
                       EXAMPLE_LCD_QSPI_V_RES,
        .rotate = LV_DISP_ROT_90,
    };

    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    // --- STEP B: BOOT SCREEN ---

    boot_screen_create();
    boot_screen_start_exit_animation();

    // Force LVGL to draw the boot screen NOW
    lv_timer_handler();

    // --- STEP C: SYSTEM INIT ---

    ESP_LOGI(TAG, "Mounting SD Card...");

    if (bsp_sd_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD Mount Failed!");
    }

    pst_file_browser_create(
        "S:",
        my_file_picker_callback);

    // --- STEP D: MAIN LOOP ---

    while (1)
    {
        uint32_t delay = lv_timer_handler();

        vTaskDelay(
            pdMS_TO_TICKS(delay > 0 ? delay : 1));
    }
}