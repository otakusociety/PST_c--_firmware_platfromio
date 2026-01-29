#include <string.h>
#include <lvgl.h>
#include "esp_log.h"
#include "esp_bsp.h"
#include "pst_keyboard.h"

#define MAX_INPUT_LEN 256

static const char *TAG = "PST_KEYBOARD";

// Keyboard layout: QWERTY, 3 rows
static const char *KEYBOARD_ROWS[] = {
    "1234567890",
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM"
};
static const int NUM_ROWS = 4;

// Static state
static lv_obj_t *s_screen = NULL;
static lv_obj_t *s_text_input = NULL;
static lv_obj_t *s_key_buttons[128] = {0};  // Array to store key button references
static char s_input_buffer[MAX_INPUT_LEN] = {0};
static pst_keyboard_done_cb_t s_done_cb = NULL;

// Helper: Update text display
static void update_text_display(void)
{
    if (s_text_input) {
        lv_textarea_set_text(s_text_input, s_input_buffer);
        // Scroll to end
        lv_textarea_set_cursor_pos(s_text_input, strlen(s_input_buffer));
    }
}

// Helper: Append character to buffer
static void append_char(char c)
{
    if (strlen(s_input_buffer) < MAX_INPUT_LEN - 1) {
        char str[2] = {c, '\0'};
        strcat(s_input_buffer, str);
        update_text_display();
        ESP_LOGI(TAG, "Appended: %c", c);
    }
}

// Helper: Delete last character
static void delete_char(void)
{
    int len = strlen(s_input_buffer);
    if (len > 0) {
        s_input_buffer[len - 1] = '\0';
        update_text_display();
        ESP_LOGI(TAG, "Deleted character");
    }
}

// Key event handler
static void key_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) {
        return;
    }

    lv_obj_t *btn = lv_event_get_target(e);
    const char *label_text = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (!label_text || strlen(label_text) == 0) {
        return;
    }

    // Check for special keys
    if (strcmp(label_text, LV_SYMBOL_BACKSPACE) == 0) {
        delete_char();
    } else if (strcmp(label_text, "ENTER") == 0) {
        if (s_done_cb) {
            s_done_cb(s_input_buffer, true);
        }
        pst_keyboard_destroy();
    } else if (strcmp(label_text, "CANCEL") == 0) {
        if (s_done_cb) {
            s_done_cb(NULL, false);
        }
        pst_keyboard_destroy();
    } else if (strcmp(label_text, "SPACE") == 0) {
        append_char(' ');
    } else if (strlen(label_text) == 1) {
        // Regular character key
        append_char(label_text[0]);
    }
}

// Create a single key button
static lv_obj_t *create_key_button(lv_obj_t *parent, const char *label, int width_unit)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_width(btn, LV_PCT(width_unit));
    lv_obj_set_height(btn, LV_SIZE_CONTENT);

    lv_obj_t *label_obj = lv_label_create(btn);
    lv_label_set_text(label_obj, label);
    lv_obj_center(label_obj);

    lv_obj_add_event_cb(btn, key_event_handler, LV_EVENT_CLICKED, NULL);

    return btn;
}

bool pst_keyboard_create(const char *prompt_text, pst_keyboard_done_cb_t on_done_cb)
{
    if (!on_done_cb) {
        ESP_LOGE(TAG, "Callback required");
        return false;
    }

    s_done_cb = on_done_cb;
    memset(s_input_buffer, 0, sizeof(s_input_buffer));

    if (!bsp_display_lock(100)) {
        ESP_LOGE(TAG, "Failed to lock display");
        return false;
    }

    // Clear screen
    s_screen = lv_scr_act();
    lv_obj_clean(s_screen);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_screen, LV_SCROLLBAR_MODE_OFF);

    // Main container (column flex)
    lv_obj_t *container = lv_obj_create(s_screen);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

    // Prompt label
    if (prompt_text && strlen(prompt_text) > 0) {
        lv_obj_t *prompt_label = lv_label_create(container);
        lv_label_set_text(prompt_label, prompt_text);
        lv_obj_add_flag(prompt_label, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    // Text input area (textarea)
    s_text_input = lv_textarea_create(container);
    lv_obj_set_width(s_text_input, LV_PCT(100));
    lv_obj_set_height(s_text_input, 60);
    lv_textarea_set_max_length(s_text_input, MAX_INPUT_LEN - 1);
    lv_textarea_set_text(s_text_input, "");
    lv_textarea_set_one_line(s_text_input, false);
    lv_textarea_set_cursor_click_pos(s_text_input, false);
    lv_obj_add_flag(s_text_input, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    // Keyboard grid container
    lv_obj_t *kb_container = lv_obj_create(container);
    lv_obj_set_width(kb_container, LV_PCT(100));
    lv_obj_set_flex_grow(kb_container, 1);
    lv_obj_set_flex_flow(kb_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(kb_container, 4, 0);
    lv_obj_set_style_border_width(kb_container, 0, 0);
    lv_obj_set_style_bg_opa(kb_container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(kb_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(kb_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(kb_container, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    // Create keyboard rows
    for (int row = 0; row < NUM_ROWS; row++) {
        lv_obj_t *row_container = lv_obj_create(kb_container);
        lv_obj_set_width(row_container, LV_PCT(100));
        lv_obj_set_height(row_container, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row_container, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_all(row_container, 0, 0);
        lv_obj_set_style_border_width(row_container, 0, 0);
        lv_obj_set_style_bg_opa(row_container, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(row_container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(row_container, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_flag(row_container, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

        const char *row_str = KEYBOARD_ROWS[row];
        int key_count = strlen(row_str);

        // Calculate button width based on row (account for margins)
        int btn_width = (100 - (key_count - 1) * 2) / key_count;  // Rough distribution
        if (btn_width < 8) btn_width = 8;

        for (int col = 0; col < key_count; col++) {
            char key_char[2] = {row_str[col], '\0'};
            create_key_button(row_container, key_char, btn_width);
        }
    }

    // Bottom action buttons row
    lv_obj_t *action_row = lv_obj_create(kb_container);
    lv_obj_set_width(action_row, LV_PCT(100));
    lv_obj_set_height(action_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(action_row, 0, 0);
    lv_obj_set_style_border_width(action_row, 0, 0);
    lv_obj_set_style_bg_opa(action_row, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(action_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(action_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(action_row, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    // Space key (wide)
    create_key_button(action_row, "SPACE", 50);

    // Backspace key
    create_key_button(action_row, LV_SYMBOL_BACKSPACE, 15);

    // Cancel button
    create_key_button(action_row, "CANCEL", 15);

    // Enter button
    create_key_button(action_row, "ENTER", 15);

    bsp_display_unlock();

    ESP_LOGI(TAG, "Keyboard created");
    return true;
}

void pst_keyboard_set_input(const char *initial_text)
{
    if (!initial_text) {
        memset(s_input_buffer, 0, sizeof(s_input_buffer));
    } else {
        strncpy(s_input_buffer, initial_text, MAX_INPUT_LEN - 1);
        s_input_buffer[MAX_INPUT_LEN - 1] = '\0';
    }

    if (!bsp_display_lock(100)) {
        ESP_LOGE(TAG, "Failed to lock display for set_input");
        return;
    }

    update_text_display();

    bsp_display_unlock();
}

void pst_keyboard_destroy(void)
{
    if (!bsp_display_lock(100)) {
        ESP_LOGW(TAG, "Failed to lock display for destroy");
        return;
    }

    if (s_screen) {
        lv_obj_clean(s_screen);
    }

    s_screen = NULL;
    s_text_input = NULL;
    memset(s_key_buttons, 0, sizeof(s_key_buttons));
    memset(s_input_buffer, 0, sizeof(s_input_buffer));
    s_done_cb = NULL;

    bsp_display_unlock();

    ESP_LOGI(TAG, "Keyboard destroyed");
}