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

#define LVGL_PORT_ROTATION_DEGREE (90)

static const char *TAG = "APP_MAIN";

// Simple hook for now: in the future this will open the editor on PST
static void on_file_selected(const char *full_path)
{
    ESP_LOGI(TAG, "Selected file for edit: %s", full_path);
}

// Keyboard callback: demonstrates integration
static void on_keyboard_done(const char *text, bool submitted)
{
    if (submitted && text)
    {
        ESP_LOGI(TAG, "Keyboard submitted: '%s'", text);
        // Future: pass text to file editor or file browser for new file creation
    }
    else
    {
        ESP_LOGI(TAG, "Keyboard cancelled");
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== SD File Browser Starting ===");

    esp_err_t ret = esp_psram_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "PSRAM init failed: %s", esp_err_to_name(ret));
        return;
    }

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
        .rotate = LV_DISP_ROT_90,
#endif
    };

    lv_disp_t *disp = bsp_display_start_with_config(&cfg);
    if (!disp)
    {
        ESP_LOGE(TAG, "Display init failed");
        return;
    }

    lv_coord_t scr_width = lv_disp_get_hor_res(disp);
    lv_coord_t scr_height = lv_disp_get_ver_res(disp);
    ESP_LOGI(TAG, "Display: %dx%d", scr_width, scr_height);

    bsp_display_backlight_on();

    vTaskDelay(pdMS_TO_TICKS(500));
    ret = bsp_sd_init();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "SD init failed: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "SD mounted: %s", bsp_sd_get_mount_point());
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    // Root the browser at the SD mount point for Edit Mode
    pst_file_browser_create(bsp_sd_get_mount_point(), on_file_selected);

    // Example: Show keyboard on startup (optional demo)
    // Uncomment to test:
    // vTaskDelay(pdMS_TO_TICKS(500));
    // pst_keyboard_create("Enter filename:", on_keyboard_done);

    while (1)
    {




}   

    lv_timer_handler();
       vTaskDelay(pdMS_TO_TICKS(5));
    
}
