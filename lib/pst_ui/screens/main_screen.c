#include "main_screen.h"
#include "../ui_events.h" // Assuming we might have a header for events later, or just declare externs here

// Declare the external event callback
extern void on_demo_btn_click(lv_event_t * e);

lv_obj_t * main_screen_create(void)
{
    // Create the screen object
    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), LV_PART_MAIN);

    // Create the Title Label
    lv_obj_t * label_title = lv_label_create(screen);
    lv_label_set_text(label_title, "PST Firmware");
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 20);
    // Note: effective font depends on what's enabled in lv_conf.h. Using default or simple one here.
    // lv_obj_set_style_text_font(label_title, &lv_font_montserrat_20, LV_PART_MAIN); 
    lv_obj_set_style_text_color(label_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create the Button
    lv_obj_t * btn = lv_btn_create(screen);
    lv_obj_set_size(btn, 160, 50);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x007bff), LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
    
    // Add Event
    lv_obj_add_event_cb(btn, on_demo_btn_click, LV_EVENT_CLICKED, NULL);

    // Button Label
    lv_obj_t * label_btn = lv_label_create(btn);
    lv_label_set_text(label_btn, "Click Me!");
    lv_obj_align(label_btn, LV_ALIGN_CENTER, 0, 0);

    return screen;
}
