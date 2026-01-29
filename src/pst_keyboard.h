/**
 * On-screen QWERTY keyboard module for PST text input.
 *
 * Responsibilities:
 *  - Render a full QWERTY keyboard layout (A-Z, 0-9, space, backspace, enter)
 *  - Maintain a text input buffer (256 char max)
 *  - Display typed text in scrollable text area
 *  - Invoke callback when user submits or cancels
 *
 * Requirements:
 *  - Display + LVGL must already be initialized via bsp_display_start_with_config()
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of callback invoked when keyboard input is complete.
 *
 * @param text       Null-terminated input string (NULL if cancelled).
 * @param submitted  True if user pressed Enter, false if user cancelled.
 */
typedef void (*pst_keyboard_done_cb_t)(const char *text, bool submitted);

/**
 * @brief Create and show the on-screen keyboard UI.
 *
 * The keyboard:
 *  - Clears the current LVGL screen
 *  - Creates a text input display area at top
 *  - Creates a QWERTY key grid below
 *  - Displays Cancel and Submit buttons
 *
 * @param prompt_text    Label above text input (e.g., "Enter filename:").
 *                       May be NULL for no label.
 * @param on_done_cb     Callback invoked when keyboard is dismissed.
 *                       Must not be NULL.
 *
 * @return true on success, false if LVGL lock fails or invalid args.
 */
bool pst_keyboard_create(const char *prompt_text, pst_keyboard_done_cb_t on_done_cb);

/**
 * @brief Pre-fill the keyboard input buffer with initial text.
 *
 * Optional: call after pst_keyboard_create() to set starting text.
 *
 * @param initial_text  Null-terminated string to pre-fill (max 255 chars).
 *                      May be NULL to clear.
 */
void pst_keyboard_set_input(const char *initial_text);

/**
 * @brief Destroy the keyboard and clean up resources.
 *
 * Safe to call even if keyboard was never created.
 */
void pst_keyboard_destroy(void);

#ifdef __cplusplus
}
#endif