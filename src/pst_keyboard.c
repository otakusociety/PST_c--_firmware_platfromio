#include <lvgl.h>
#include "esp_log.h"
#include "esp_bsp.h"
#include "pst_keyboard.h"

static const char *TAG = "PST_KEYBOARD";

static lv_obj_t *s_modal_base = NULL; // Background dimming/click catcher
static lv_obj_t *s_kb = NULL;
static lv_obj_t *s_ta = NULL;
static pst_keyboard_done_cb_t s_done_cb = NULL;

static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY)
    {
        if (s_done_cb)
            s_done_cb(lv_textarea_get_text(s_ta), true);
        pst_keyboard_destroy();
    }
    else if (code == LV_EVENT_CANCEL)
    {
        if (s_done_cb)
            s_done_cb(NULL, false);
        pst_keyboard_destroy();
    }
}

// Dismiss keyboard if user touches the background dim area
static void modal_click_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        if (s_done_cb)
            s_done_cb(NULL, false);
        pst_keyboard_destroy();
    }
}

bool pst_keyboard_create(const char *prompt_text, pst_keyboard_done_cb_t on_done_cb)
{
    s_done_cb = on_done_cb;
    if (!bsp_display_lock(100))
        return false;

    // DO NOT clean the screen here!

    // 1. Create a Modal Background (Dimming effect)
    s_modal_base = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_modal_base, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_modal_base, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_modal_base, LV_OPA_50, 0); // Dim the background
    lv_obj_set_style_border_width(s_modal_base, 0, 0);
    lv_obj_set_style_radius(s_modal_base, 0, 0);
    lv_obj_add_event_cb(s_modal_base, modal_click_cb, LV_EVENT_CLICKED, NULL);

    // 2. Create Text Area inside a small container
    s_ta = lv_textarea_create(s_modal_base);
    lv_obj_set_size(s_ta, LV_PCT(90), 45);
    lv_obj_align(s_ta, LV_ALIGN_TOP_MID, 0, 20);
    lv_textarea_set_placeholder_text(s_ta, prompt_text ? prompt_text : "Search...");
    lv_textarea_set_one_line(s_ta, true);
    lv_obj_add_state(s_ta, LV_STATE_FOCUSED); // Auto-focus

    // 3. Create Keyboard
    s_kb = lv_keyboard_create(s_modal_base);
    lv_obj_set_size(s_kb, LV_PCT(100), LV_PCT(55));
    lv_obj_align(s_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(s_kb, s_ta);
    lv_obj_add_event_cb(s_kb, kb_event_cb, LV_EVENT_ALL, NULL);

    bsp_display_unlock();
    return true;
}

void pst_keyboard_destroy(void)
{
    if (bsp_display_lock(100))
    {
        if (s_modal_base)
        {
            lv_obj_del(s_modal_base); // Deleting the base deletes the TA and KB too!
            s_modal_base = NULL;
            s_kb = NULL;
            s_ta = NULL;
        }
        bsp_display_unlock();
    }
}