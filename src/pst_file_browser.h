/**
 * Simple LVGL-based SD card file browser module for PST.
 *
 * Responsibilities:
 *  - Render a scrollable list of files/directories rooted at a base path (e.g. "/sd")
 *  - Keep a persistent Back button/header always visible
 *  - Expose a callback when a file (not directory) is selected, with full path
 *
 * Requirements:
 *  - Display + LVGL must already be initialized via bsp_display_start_with_config()
 *  - SD card should be mounted via bsp_sd_init()
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of callback invoked when the user selects a file.
 *
 * @param full_path  Null-terminated absolute path to the selected file.
 */
typedef void (*pst_file_selected_cb_t)(const char *full_path);

/**
 * @brief Create and show the PST file browser UI.
 *
 * The browser:
 *  - Clears the current LVGL screen
 *  - Creates a fixed Back header and a scrollable file list
 *  - Starts at the given root path (e.g. "/sd")
 *
 * @param root_path      Base directory the browser is constrained to.
 *                       Typically the SD mount point from bsp_sd_get_mount_point().
 * @param on_file_cb     Callback invoked when a regular file is tapped.
 *                       May be NULL if you only care about navigation.
 *
 * @return true on success, false if LVGL lock fails.
 */
bool pst_file_browser_create(const char *root_path, pst_file_selected_cb_t on_file_cb);

/**
 * @brief Programmatically change root path and refresh browser.
 *
 * @param root_path  New root directory.
 */
void pst_file_browser_set_root(const char *root_path);

#ifdef __cplusplus
}
#endif


