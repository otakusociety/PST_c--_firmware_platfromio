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

static const char *TAG = "EXPLORER_TEST";

// This callback runs when you pick a file in the explorer
void my_file_picker_callback(const char *full_path)
{
    if (full_path)
    {
        ESP_LOGI(TAG, "SUCCESS! You selected: %s", full_path);
        // Here you would typically open the file or play the music/image
    }
    else
    {
        ESP_LOGW(TAG, "File selection was cancelled or failed.");
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing System...");

    // 1. Initialize the Display
    // Using default BSP config, adjust rotation if your screen is upside down
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
        .rotate = LV_DISP_ROT_90,
    };
    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    // 2. Initialize the SD Card (CRITICAL)
    // The File Explorer will show an empty list if the SD isn't mounted
    ESP_LOGI(TAG, "Mounting SD Card...");
    esp_err_t ret = bsp_sd_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card! Check your wiring/card.");
    }

    // 3. Launch the Browser
    // "S:" is the drive letter we set in lv_conf.h
    ESP_LOGI(TAG, "Launching File Browser...");
    pst_file_browser_create("S:", my_file_picker_callback);

    // 4. Main Loop
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