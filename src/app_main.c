#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include "esp_bsp.h"
#include "display.h"
#include "lv_port.h"
#include "esp_psram.h"
#include "esp_log.h"

#define LVGL_PORT_ROTATION_DEGREE (90)
#define SIZE_X 50
#define SIZE_Y 50
#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 480

static const char *TAG = "APP_MAIN";
static lv_obj_t *g_slider = NULL;

static void box_event_handler(lv_event_t *e)
{
    lv_obj_t *box = lv_event_get_target(e);
    lv_color_t color = lv_obj_get_style_bg_color(box, 0);

    if (g_slider) {
        lv_obj_set_style_bg_color(g_slider, color, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(g_slider, color, LV_PART_KNOB);
    }
}

static lv_obj_t *create_colored_box(lv_obj_t *parent, lv_color_t color, const char *label_text)
{
    lv_obj_t *rect = lv_obj_create(parent);
    lv_obj_set_size(rect, SIZE_X, SIZE_Y);
    lv_obj_set_style_bg_color(rect, color, 0);
    lv_obj_set_style_border_width(rect, 2, 0);
    lv_obj_set_style_border_color(rect, lv_color_white(), 0);

    lv_obj_t *label = lv_label_create(rect);
    lv_label_set_text(label, label_text);
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);

    lv_obj_add_event_cb(rect, box_event_handler, LV_EVENT_CLICKED, NULL);

    return rect;
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== PST Display Initialization ===");
    
    // 1. PSRAM Init
    esp_err_t ret = esp_psram_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PSRAM init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "✓ PSRAM initialized");

    // 2. Display Config
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
        .rotate = LV_DISP_ROT_90,
#endif
    };

    ESP_LOGI(TAG, "Starting display with rotation LV_DISP_ROT_90");
    lv_disp_t *disp = bsp_display_start_with_config(&cfg);
    
    if (!disp) {
        ESP_LOGE(TAG, "Display initialization FAILED");
        return;
    }
    
    lv_coord_t scr_width = lv_disp_get_hor_res(disp);
    lv_coord_t scr_height = lv_disp_get_ver_res(disp);
    ESP_LOGI(TAG, "✓ Display initialized: %d×%d", scr_width, scr_height);

    ret = bsp_display_backlight_on();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Backlight on failed: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "✓ Backlight ON");

    // 3. UI Construction (under lock)
    if (!bsp_display_lock(100)) {
        ESP_LOGE(TAG, "Failed to acquire display lock");
        return;
    }
    ESP_LOGI(TAG, "Building UI...");

    lv_obj_t *scr = lv_scr_act();
    if (!scr) {
        ESP_LOGE(TAG, "Failed to get active screen");
        bsp_display_unlock();
        return;
    }

    // Disable scrolling completely
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(scr, LV_DIR_NONE);
    lv_obj_set_height(scr, lv_pct(100));
    lv_obj_set_width(scr, lv_pct(100));
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    // Slider (centered)
    g_slider = lv_slider_create(scr);
    lv_obj_set_size(g_slider, 200, 40);
    lv_obj_align(g_slider, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(g_slider, lv_color_make(128, 128, 128), 0);
    ESP_LOGI(TAG, "✓ Slider created");

    // Corner boxes
    lv_obj_t *box_tl = create_colored_box(scr, lv_color_make(255, 0, 0), "R");
    lv_obj_align(box_tl, LV_ALIGN_TOP_LEFT, 0, 0);
    
    lv_obj_t *box_tr = create_colored_box(scr, lv_color_make(0, 255, 0), "G");
    lv_obj_align(box_tr, LV_ALIGN_TOP_RIGHT, 0, 0);
    
    lv_obj_t *box_bl = create_colored_box(scr, lv_color_make(0, 0, 255), "B");
    lv_obj_align(box_bl, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    
    lv_obj_t *box_br = create_colored_box(scr, lv_color_white(), "W");
    lv_obj_align(box_br, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    
    ESP_LOGI(TAG, "✓ Corner boxes created");

    // Diagonal line: must position line object, then define points relative to it
    // Line spans from top-left corner center to bottom-right corner center
    lv_obj_t *line = lv_line_create(scr);
    
    // Position line object at top-left corner of screen
    lv_obj_set_pos(line, 0, 0);
    
    // Define points relative to line object (0,0) = screen top-left
    static lv_point_t line_pts[2];
    line_pts[0] = (lv_point_t){SIZE_X / 2, SIZE_Y / 2};                              // Center of TL box
    line_pts[1] = (lv_point_t){scr_width - SIZE_X / 2, scr_height - SIZE_Y / 2};    // Center of BR box
    
    lv_line_set_points(line, line_pts, 2);
    lv_obj_set_style_line_color(line, lv_color_white(), 0);
    lv_obj_set_style_line_width(line, 2, 0);
    
    ESP_LOGI(TAG, "✓ Diagonal line created: (%.0f,%.0f)→(%.0f,%.0f)",
             (float)line_pts[0].x, (float)line_pts[0].y,
             (float)line_pts[1].x, (float)line_pts[1].y);

    bsp_display_unlock();
    ESP_LOGI(TAG, "✓ UI built successfully");

    // 4. Main loop - yield frequently to LVGL task
    ESP_LOGI(TAG, "Entering main loop...");
    while (1) {
        // Yield every 5ms to allow LVGL task to run
        // LVGL's internal task handles rendering, not main loop
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}