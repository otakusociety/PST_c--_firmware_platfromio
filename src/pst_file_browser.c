#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_bsp.h"
#include "pst_file_browser.h"
#include "pst_keyboard.h"

static const char *TAG = "PST_MODERN_FS";

static lv_obj_t *s_list = NULL;
static lv_obj_t *s_path_label = NULL;
static lv_obj_t *s_main_cont = NULL; // Main container
static char s_current_path[256] = "S:";
static char s_filter[64] = "";
static char s_selected_file[256] = "";
static pst_file_selected_cb_t s_file_cb = NULL;

static void refresh_list(void);

// Automatically clear the static pointer when LVGL destroys the main container
static void main_cont_delete_cb(lv_event_t *e)
{
    s_main_cont = NULL;
    s_list = NULL;
    s_path_label = NULL;
}

// Safely clean up allocated string memory when a list button is destroyed
static void btn_delete_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    char *name = (char *)lv_obj_get_user_data(obj);
    if (name)
    {
        lv_mem_free(name);
        lv_obj_set_user_data(obj, NULL);
    }
}

static void on_search_finished(const char *text, bool submitted)
{
    if (submitted && text)
    {
        snprintf(s_filter, sizeof(s_filter), "%s", text);
    }
    else
    {
        s_filter[0] = '\0';
    }

    if (bsp_display_lock(100))
    {
        refresh_list();
        bsp_display_unlock();
    }
}

static void header_click_event_handler(lv_event_t *e)
{
    static uint32_t last_click = 0;
    if (lv_tick_get() - last_click < 500)
    {
        return;
    }
    last_click = lv_tick_get();

    pst_keyboard_create("Search files:", on_search_finished);
}

static void list_btn_event_handler(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(s_list, btn);
    char *clean_name = (char *)lv_obj_get_user_data(btn);

    static char new_path[512];

    if (strcmp(btn_text, LV_SYMBOL_UP " ..") == 0)
    {
        s_filter[0] = '\0';
        char *last_slash = strrchr(s_current_path, '/');
        if (last_slash != NULL && last_slash > s_current_path + 2)
        {
            *last_slash = '\0';
        }
        else
        {
            strcpy(s_current_path, "S:");
        }
    }
    else
    {
        if (!clean_name)
            return;

        snprintf(new_path, sizeof(new_path), "%s/%s", s_current_path, clean_name);

        // Quick check using LVGL internal flag or directory test
        lv_fs_dir_t dir;
        if (lv_fs_dir_open(&dir, new_path) == LV_FS_RES_OK)
        {
            lv_fs_dir_close(&dir);
            strncpy(s_current_path, new_path, sizeof(s_current_path) - 1);
            s_current_path[sizeof(s_current_path) - 1] = '\0';
            s_filter[0] = '\0';
        }
        else
        {
            // It's a file
            strncpy(s_selected_file, new_path, sizeof(s_selected_file) - 1);
            s_selected_file[sizeof(s_selected_file) - 1] = '\0';

            if (s_file_cb)
            {
                s_file_cb(new_path);
            }
            return;
        }
    }
    refresh_list();
}

static void refresh_list(void)
{
    if (!s_list)
        return;

    // Deletes child buttons; triggers btn_delete_event_cb to free name strings
    lv_obj_clean(s_list);

    lv_obj_t *header_obj = lv_obj_get_parent(s_path_label);
    if (s_filter[0] != '\0')
    {
        lv_label_set_text_fmt(s_path_label, "Filter: %s", s_filter);
        lv_obj_set_style_bg_color(header_obj, lv_palette_main(LV_PALETTE_TEAL), 0);
    }
    else
    {
        lv_label_set_text(s_path_label, s_current_path);
        lv_obj_set_style_bg_color(header_obj, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
    }

    if (strcmp(s_current_path, "S:") != 0)
    {
        lv_obj_t *btn = lv_list_add_btn(s_list, NULL, LV_SYMBOL_UP " ..");
        lv_obj_add_event_cb(btn, list_btn_event_handler, LV_EVENT_CLICKED, NULL);
    }

    lv_fs_dir_t dir;
    if (lv_fs_dir_open(&dir, s_current_path) != LV_FS_RES_OK)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", s_current_path);
        return;
    }

    char fn[256];
    uint32_t items_found = 0;
    while (lv_fs_dir_read(&dir, fn) == LV_FS_RES_OK)
    {
        if (fn[0] == '\0')
            break;

        bool is_dir = (fn[0] == '/');
        const char *entry_name = is_dir ? &fn[1] : fn;

        if (s_filter[0] != '\0' && strcasestr(entry_name, s_filter) == NULL)
        {
            continue;
        }

        items_found++;
        lv_obj_t *btn = lv_list_add_btn(s_list, is_dir ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, entry_name);

        char *name_copy = (char *)lv_mem_alloc(strlen(entry_name) + 1);
        if (name_copy)
        {
            strcpy(name_copy, entry_name);
            lv_obj_set_user_data(btn, name_copy);
            lv_obj_add_event_cb(btn, btn_delete_event_cb, LV_EVENT_DELETE, NULL);
        }

        if (is_dir)
        {
            lv_obj_set_style_text_color(btn, lv_palette_main(LV_PALETTE_AMBER), 0);
        }

        lv_obj_add_event_cb(btn, list_btn_event_handler, LV_EVENT_CLICKED, NULL);
    }
    lv_fs_dir_close(&dir);

    if (items_found == 0 && s_filter[0] != '\0')
    {
        lv_obj_t *empty_info = lv_list_add_text(s_list, "No files match your search.");
        lv_obj_set_style_text_align(empty_info, LV_TEXT_ALIGN_CENTER, 0);
    }
}

bool pst_file_browser_create(const char *root_path, pst_file_selected_cb_t on_file_cb)
{
    s_file_cb = on_file_cb;

    if (!bsp_display_lock(100))
    {
        return false;
    }

    lv_obj_t *scr = lv_scr_act();

    // Re-create or reuse container safely
    if (s_main_cont == NULL)
    {
        s_main_cont = lv_obj_create(scr);
        lv_obj_set_size(s_main_cont, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_pad_all(s_main_cont, 0, 0);
        lv_obj_set_style_border_width(s_main_cont, 0, 0);
        lv_obj_set_style_radius(s_main_cont, 0, 0);

        // Track lifecycle so s_main_cont doesn't dangle if external code cleans screen
        lv_obj_add_event_cb(s_main_cont, main_cont_delete_cb, LV_EVENT_DELETE, NULL);
    }

    lv_obj_clean(s_main_cont);

    // Header Button (Acts as Search Bar trigger)
    lv_obj_t *header = lv_btn_create(s_main_cont);
    lv_obj_set_size(header, LV_PCT(100), 45);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(header, header_click_event_handler, LV_EVENT_CLICKED, NULL);

    s_path_label = lv_label_create(header);
    lv_obj_align(s_path_label, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_color(s_path_label, lv_color_white(), 0);

    // File list widget
    s_list = lv_list_create(s_main_cont);
    lv_obj_set_size(s_list, LV_PCT(95), LV_PCT(82));
    lv_obj_align(s_list, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_radius(s_list, 10, 0);

    if (root_path && root_path[0] != '\0')
    {
        strncpy(s_current_path, root_path, sizeof(s_current_path) - 1);
        s_current_path[sizeof(s_current_path) - 1] = '\0';
    }

    refresh_list();

    bsp_display_unlock();
    return true;
}

void pst_file_browser_set_root(const char *root_path)
{
    if (!root_path || root_path[0] == '\0')
        return;

    if (bsp_display_lock(100))
    {
        strncpy(s_current_path, root_path, sizeof(s_current_path) - 1);
        s_current_path[sizeof(s_current_path) - 1] = '\0';
        s_filter[0] = '\0';
        refresh_list();
        bsp_display_unlock();
    }
}