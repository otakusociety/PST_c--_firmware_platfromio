/**
 * @file screen_file_browser_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "screen_file_browser_gen.h"
#include "pst_ui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * screen_file_browser_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    static bool style_inited = false;

    if (!style_inited) {

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    //lv_obj_set_name_static(lv_obj_0, "screen_file_browser_#");
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(lv_obj_0, 16, 0);
    lv_obj_set_style_bg_color(lv_obj_0, lv_color_hex(0xffffff), 0);

    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_label_bind_text(lv_label_0, &selected_file, NULL);
    lv_obj_set_style_text_color(lv_label_0, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lv_label_0, font_medium, 0);
    
    lv_obj_t * lv_button_0 = lv_button_create(lv_obj_0);
    lv_obj_set_width(lv_button_0, 200);
    lv_obj_set_height(lv_button_0, 50);
    lv_obj_set_style_bg_color(lv_button_0, lv_color_hex(0x0077ff), 0);
    lv_obj_set_style_radius(lv_button_0, 8, 0);
    lv_obj_t * lv_label_1 = lv_label_create(lv_button_0);
    lv_label_set_text(lv_label_1, "Open File Browser");
    lv_obj_set_style_text_color(lv_label_1, lv_color_hex(0xffffff), 0);
    
    lv_obj_add_event_cb(lv_button_0, my_file_picker_callback, LV_EVENT_CLICKED, NULL);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

