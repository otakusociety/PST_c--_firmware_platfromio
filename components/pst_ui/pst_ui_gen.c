/**
 * @file pst_ui_gen.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "pst_ui_gen.h"

#if LV_USE_XML
#endif /* LV_USE_XML */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/*----------------
 * Translations
 *----------------*/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/

lv_font_t * font_medium;
extern lv_font_t font_medium_data;

/*----------------
 * Images
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

lv_subject_t selected_file;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void pst_ui_init_gen(const char * asset_path)
{
    char buf[256];

    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Fonts
     *----------------*/

    /* get font 'font_medium' from a C array */
    font_medium = &font_medium_data;


    /*----------------
     * Images
     *----------------*/
    /*----------------
     * Subjects
     *----------------*/
    static char selected_file_buf[UI_SUBJECT_STRING_LENGTH];
    static char selected_file_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&selected_file,
                           selected_file_buf,
                           selected_file_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "No File Selected"
                          );

    /*----------------
     * Translations
     *----------------*/

#if LV_USE_XML
    /* Register widgets */

    /* Register fonts */
    lv_xml_register_font(NULL, "font_medium", font_medium);

    /* Register subjects */
    lv_xml_register_subject(NULL, "selected_file", &selected_file);

    /* Register callbacks */
    lv_xml_register_event_cb(NULL, "my_file_picker_callback", my_file_picker_callback);
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)
    /* Register images */
#endif

#if LV_USE_XML == 0
    /*--------------------
     *  Permanent screens
     *-------------------*/
    /* If XML is enabled it's assumed that the permanent screens are created
     * manaully from XML using lv_xml_create() */
#endif
}

/* Callbacks */
#if defined(LV_EDITOR_PREVIEW)
void __attribute__((weak)) my_file_picker_callback(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("my_file_picker_callback was called\n");
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/