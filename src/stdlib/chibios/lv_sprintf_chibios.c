/**
 * @file lv_sprintf_chibios.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"
#if LV_USE_STDLIB_SPRINTF == LV_STDLIB_CHIBIOS
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int lv_snprintf(char * buffer, size_t count, const char * format, ...) {
  va_list va;
  va_start(va, format);
  const int ret = chsnprintf(buffer, count, format, va);
  va_end(va);
  return ret;
}

int lv_vsnprintf(char * buffer, size_t count, const char * format, va_list va) {
  return chvsnprintf(buffer, count, format, va);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_STDLIB_RTTHREAD*/
