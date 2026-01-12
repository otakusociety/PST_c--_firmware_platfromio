#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include "esp_bsp.h"
#include "display.h"
#include "lv_port.h"
#include "esp_psram.h"              // ← Add this
#include "esp_log.h"                // ← Add this

#define LVGL_PORT_ROTATION_DEGREE (90)

void app_main(void)
{
      printf("Starting JC3248W535EN LVGL Demo\n");

      // Initialize PSRAM explicitly
      esp_err_t ret = esp_psram_init();
      if (ret != ESP_OK) {
            ESP_LOGE("PSRAM", "PSRAM init failed: %s", esp_err_to_name(ret));
            return;
      }
      ESP_LOGI("PSRAM", "PSRAM initialized successfully");

      
      
      bsp_display_cfg_t cfg = {
          .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
          .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
          .rotate = LV_DISP_ROT_90,
#endif
      };

      bsp_display_start_with_config(&cfg);
      bsp_display_backlight_on();

      bsp_display_lock(0);

      lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
      lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

      lv_obj_t *label = lv_label_create(btn1);
      lv_label_set_text(label, "Button");
      lv_obj_center(label);

      bsp_display_unlock();

      while (1)
      {
            lv_timer_handler();
            vTaskDelay(pdMS_TO_TICKS(5));
      }
}
