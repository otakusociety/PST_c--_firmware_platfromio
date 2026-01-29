#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lvgl.h>

#include "esp_log.h"
#include "esp_bsp.h"
#include "pst_file_browser.h"

#define MAX_FILES_PER_DIR 100

static const char *TAG = "PST_FILE_BROWSER";

static lv_obj_t *s_file_list = NULL;
static lv_obj_t *s_back_btn = NULL;
static char s_root_path[256] = "/sd";
static char s_current_path[256] = "/sd";
static pst_file_selected_cb_t s_file_cb = NULL;

static void file_browser_refresh(const char *path);
static void file_item_event_handler(lv_event_t *e);

static void set_current_path(const char *path)
{
    strncpy(s_current_path, path, sizeof(s_current_path) - 1);
    s_current_path[sizeof(s_current_path) - 1] = '\0';
}

bool pst_file_browser_create(const char *root_path, pst_file_selected_cb_t on_file_cb)
{
    if (root_path && root_path[0] != '\0') {
        strncpy(s_root_path, root_path, sizeof(s_root_path) - 1);
        s_root_path[sizeof(s_root_path) - 1] = '\0';
    }
    set_current_path(s_root_path);
    s_file_cb = on_file_cb;

    if (!bsp_display_lock(100)) {
        ESP_LOGE(TAG, "Failed to lock display");
        return false;
    }

    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);

    // Disable scrolling on the base screen so only the inner list can scroll
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    // Container with column flex: [Back header] + [scrolling list]
    lv_obj_t *container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

    // Fixed Back button (never scrolls)
    s_back_btn = lv_btn_create(container);
    lv_obj_t *back_label = lv_label_create(s_back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
    lv_obj_add_event_cb(s_back_btn, file_item_event_handler, LV_EVENT_CLICKED, NULL);

    // Scrollable file list fills the rest
    s_file_list = lv_list_create(container);
    lv_obj_set_flex_grow(s_file_list, 1);

    bsp_display_unlock();

    file_browser_refresh(s_current_path);
    return true;
}

void pst_file_browser_set_root(const char *root_path)
{
    if (!root_path || root_path[0] == '\0') {
        return;
    }
    strncpy(s_root_path, root_path, sizeof(s_root_path) - 1);
    s_root_path[sizeof(s_root_path) - 1] = '\0';
    set_current_path(s_root_path);
    file_browser_refresh(s_current_path);
}

static void file_browser_refresh(const char *path)
{
    ESP_LOGI(TAG, "Refreshing: %s", path);

    if (!bsp_display_lock(100)) {
        ESP_LOGE(TAG, "Lock failed");
        return;
    }

    if (!s_file_list) {
        bsp_display_unlock();
        return;
    }

    if (!bsp_sd_is_mounted()) {
        lv_obj_clean(s_file_list);
        lv_list_add_btn(s_file_list, LV_SYMBOL_WARNING, "SD Not Mounted");
        bsp_display_unlock();
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        ESP_LOGE(TAG, "Cannot open: %s", path);
        bsp_display_unlock();
        return;
    }

    lv_obj_clean(s_file_list);

    struct dirent *entry;
    int file_count = 0;
    while ((entry = readdir(dir)) && file_count < MAX_FILES_PER_DIR) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) == 0) {
            const char *symbol = S_ISDIR(st.st_mode) ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE;
            lv_obj_t *btn = lv_list_add_btn(s_file_list, symbol, entry->d_name);
            lv_obj_add_event_cb(btn, file_item_event_handler, LV_EVENT_CLICKED, NULL);
            file_count++;
        }
    }
    closedir(dir);

    // Ensure list is scrolled to top
    lv_obj_scroll_to_y(s_file_list, 0, LV_ANIM_OFF);

    bsp_display_unlock();
    ESP_LOGI(TAG, "Refreshed %d items", file_count);
}

static void navigate_up(void)
{
    // Don't go above root
    if (strcmp(s_current_path, s_root_path) == 0) {
        return;
    }

    char *last_slash = strrchr(s_current_path, '/');
    if (last_slash && last_slash != s_current_path) {
        *last_slash = '\0';
    } else {
        // Fallback: reset to root
        set_current_path(s_root_path);
    }
}

static void file_item_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    lv_obj_t *btn = lv_event_get_target(e);
    bool do_refresh = true;

    if (btn == s_back_btn) {
        navigate_up();
    } else {
        // List entry: use its label as item name
        lv_obj_t *label = lv_obj_get_child(btn, 1); // list btn: [icon, label]
        if (!label) {
            return;
        }

        const char *item_name = lv_label_get_text(label);
        ESP_LOGI(TAG, "Clicked: %s", item_name);

        char new_path[512];
        snprintf(new_path, sizeof(new_path), "%s/%s", s_current_path, item_name);
        struct stat st;
        if (bsp_sd_is_mounted() && stat(new_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            set_current_path(new_path);
        } else {
            ESP_LOGI(TAG, "File: %s", new_path);
            do_refresh = false;
            if (s_file_cb) {
                s_file_cb(new_path);
            }
        }
    }

    if (do_refresh) {
        vTaskDelay(pdMS_TO_TICKS(10));
        file_browser_refresh(s_current_path);
    }
}


