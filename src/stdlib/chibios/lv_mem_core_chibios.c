/**
 * @file lv_malloc_core_chibios.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_mem.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CHIBIOS

#include "ch.h"
#include "hal.h"
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

void *chHeapRealloc(void *addr, size_t size);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize to use malloc/free/realloc etc
 */
void lv_mem_init(void) {
  /* Nothing to init. */
  return;
}

/**
 * Drop all dynamically allocated memory and reset the memory pools' state
 */
void lv_mem_deinit(void) {
  /* Nothing to deinit. */
  return;
}

lv_mem_pool_t lv_mem_add_pool(void * mem, size_t bytes) {
  /* Not supported. */
  LV_UNUSED(mem);
  LV_UNUSED(bytes);
  return NULL;
}

void lv_mem_remove_pool(lv_mem_pool_t pool) {
  /* Not supported. */
  LV_UNUSED(pool);
  return;
}

/**
 * Used internally to execute a plain `malloc` operation
 * @param size      size in bytes to `malloc`
 */
void * lv_malloc_core(size_t size) {
  return chHeapAlloc(NULL, size);
}

/**
 * Used internally to execute a plain realloc operation
 * @param p         memory address to realloc
 * @param new_size  size in bytes to realloc
 */
void * lv_realloc_core(void * p, size_t new_size) {
  return chHeapRealloc(p, new_size);
}

/**
 * Used internally to execute a plain `free` operation
 * @param p      memory address to free
 */
void lv_free_core(void * p) {
  chHeapFree(p);
}

/**
 * Used internally by lv_mem_monitor() to gather LVGL heap state information.
 * @param mon_p      pointer to lv_mem_monitor_t object to be populated.
 */
void lv_mem_monitor_core(lv_mem_monitor_t * mon_p) {
  /* Not supported. */
  LV_UNUSED(mon_p);
  return;
}

lv_result_t lv_mem_test_core(void) {
  /* Not supported. */
  return LV_RESULT_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void *chHeapRealloc(void *addr, size_t size) {
  union heap_header *hp;
  uint32_t prev_size, new_size;

  void *ptr;

  if(addr == NULL) {
    return chHeapAlloc(NULL, size);
  }

  /* previous allocated segment is preceded by an heap_header */
  hp = addr - sizeof(union heap_header);
  prev_size = hp->used.size; /* size is always multiple of 8 */

  /* check new size memory alignment */
  if(size % 8 == 0) {
    new_size = size;
  }
  else {
    new_size = ((int) (size / 8)) * 8 + 8;
  }

  if(prev_size >= new_size) {
    return addr;
  }

  ptr = chHeapAlloc(NULL, size);
  if(ptr == NULL) {
    return NULL;
  }

  memcpy(ptr, addr, prev_size);

  chHeapFree(addr);

  return ptr;
}

#endif /*LV_STDLIB_CHIBIOS*/
