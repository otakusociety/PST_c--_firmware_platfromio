# 1. Only grab the main code file
idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES pst_ui
)