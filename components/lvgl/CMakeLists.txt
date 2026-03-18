file(GLOB_RECURSE LVGL_SOURCES
    src/*.c
)

idf_component_register(
    SRCS ${LVGL_SOURCES}
    INCLUDE_DIRS "." "src"
)