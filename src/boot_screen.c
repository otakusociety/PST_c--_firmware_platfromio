

#include "boot_screen.h"
#include <stdint.h>
#include <lvgl.h>
#include "esp_log.h"

static const char *TAG = "BOOT_SCREEN";

static lv_obj_t *boot_screen = NULL;

static void anim_ready_cb(lv_anim_t *a)
{
    if (boot_screen != NULL)
    {
        lv_obj_del(boot_screen);
        boot_screen = NULL;
    }

    ESP_LOGI(TAG, "Boot screen removed.");
}

static void opa_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

void boot_screen_create(void)
{
    boot_screen = lv_obj_create(lv_layer_top());

    lv_obj_set_size(
        boot_screen,
        LV_PCT(100),
        LV_PCT(100));

    lv_obj_set_style_bg_color(
        boot_screen,
        lv_color_hex(0x1A1A1A),
        0);

    lv_obj_set_style_border_width(boot_screen, 0, 0);
    lv_obj_set_style_radius(boot_screen, 0, 0);

    lv_obj_t *label = lv_label_create(boot_screen);

    lv_label_set_text(label, "PST Getting Ready...");

    lv_obj_set_style_text_font(
        label,
        &lv_font_montserrat_14,
        0);

    lv_obj_set_style_text_color(
        label,
        lv_color_white(),
        0);

    lv_obj_center(label);

    lv_obj_t *bar = lv_bar_create(boot_screen);

    lv_obj_set_size(bar, 150, 10);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 40);

    lv_bar_set_value(bar, 100, LV_ANIM_ON);
}

void boot_screen_start_exit_animation(void)
{
    if (boot_screen == NULL)
    {
        return;
    }

    lv_anim_t a;

    lv_anim_init(&a);
    lv_anim_set_var(&a, boot_screen);

    lv_anim_set_values(
        &a,
        LV_OPA_COVER,
        LV_OPA_TRANSP);

    lv_anim_set_time(&a, 1000);
    lv_anim_set_delay(&a, 2000);

    lv_anim_set_exec_cb(&a, opa_anim_cb);
    lv_anim_set_ready_cb(&a, anim_ready_cb);

    lv_anim_start(&a);
}