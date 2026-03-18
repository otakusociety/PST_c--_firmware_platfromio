# 1. Pull in the list of .c files from your CLI export
include(${CMAKE_CURRENT_LIST_DIR}/file_list_gen.cmake)

# 2. Check if we are in the LVGL Pro Editor Preview or the actual ESP32 project
if (LV_EDITOR_PREVIEW)
    # --- EDITOR PREVIEW MODE (Generic CMake) ---
    add_library(lib-ui ${PROJECT_SOURCES})
    target_include_directories(lib-ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    # Add subfolders so the previewer can find headers
    target_include_directories(lib-ui PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/screens
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/components
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        ${CMAKE_CURRENT_SOURCE_DIR}/images
    )
else ()
    # --- ESP32 PROJECT MODE (ESP-IDF) ---
    idf_component_register(
        SRCS ${PROJECT_SOURCES}
        INCLUDE_DIRS "." "screens" "widgets" "components" "fonts"
        PRIV_REQUIRES lvgl
    )
endif ()