/**
 * @file lv_freertos.c
 *
 */

/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*********************
 *      INCLUDES
 *********************/
#include "lv_os.h"
#if LV_USE_OS == LV_OS_CHIBIOS

#include "ch.h"
#include "hal.h"

#include "../misc/lv_log.h"

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

/**
 * Create a new thread
 * @param thread        a variable in which the thread will be stored
 * @param name          the name of the thread
 * @param prio          priority of the thread
 * @param callback      function of the thread
 * @param stack_size    stack size in bytes
 * @param user_data     arbitrary data, will be available in the callback
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_thread_init(lv_thread_t * thread, const char * const name,
                           lv_thread_prio_t prio, void (*callback)(void *), size_t stack_size,
                           void * user_data) {

  thread->thread = chThdCreateFromHeap(NULL, stack_size, name, (tprio_t)prio, (tfunc_t)callback, user_data);

  /* Ensure that the ChibiOS thread was successfully created. */
  if(thread->thread == NULL) {
      LV_LOG_ERROR("chThdCreateFromHeap failed!");
      return LV_RESULT_INVALID;
  }
  return LV_RESULT_OK;
}

/**
 * Delete a thread
 * @param thread        the thread to delete
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 *
 * @note TODO: the thread must have chThdShouldTerminateX() as the while condition
 */
lv_result_t lv_thread_delete(lv_thread_t * thread) {

  chThdTerminate(thread->thread);
  msg_t status = chThdWait(thread->thread);

  if (status != MSG_OK) {
    return LV_RESULT_INVALID;
  }
  return LV_RESULT_OK;
}

/**
 * Create a mutex
 * @param mutex         a variable in which the thread will be stored
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_mutex_init(lv_mutex_t * mutex) {
  chMtxObjectInit(&mutex->mutex);
  return LV_RESULT_OK;
}

/**
 * Lock a mutex
 * @param mutex         the mutex to lock
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_mutex_lock(lv_mutex_t * mutex) {
  chMtxLock(&mutex->mutex);
  return LV_RESULT_OK;
}

/**
 * Lock a mutex from interrupt
 * @param mutex         the mutex to lock
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_mutex_lock_isr(lv_mutex_t * mutex) {
  chMtxLockS(&mutex->mutex);
  return LV_RESULT_OK;
}

/**
 * Unlock a mutex
 * @param mutex         the mutex to unlock
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_mutex_unlock(lv_mutex_t * mutex) {
  chMtxUnlock(&mutex->mutex);
  return LV_RESULT_OK;
}

/**
 * Delete a mutex
 * @param mutex         the mutex to delete
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_mutex_delete(lv_mutex_t * mutex) {
  chMtxObjectDispose(&mutex->mutex);
  return LV_RESULT_OK;
}

/* TODO: semafori? */
/**
 * Create a thread synchronization object
 * @param sync          a variable in which the sync will be stored
 * @return              LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_thread_sync_init(lv_thread_sync_t * sync) {
  chSemObjectInit(&sync->sem, 0);
  return LV_RESULT_OK;
}

/**
 * Wait for a "signal" on a sync object
 * @param sync      a sync object
 * @return          LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_thread_sync_wait(lv_thread_sync_t * sync) {
  if (chSemWait(&sync->sem) != MSG_OK) {
    return LV_RESULT_INVALID;
  }
  else {
    return LV_RESULT_OK;
  }
}

/**
 * Send a wake-up signal to a sync object
 * @param sync      a sync object
 * @return          LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_thread_sync_signal(lv_thread_sync_t * sync) {
  chSemSignal(&sync->sem);
  return LV_RESULT_OK;
}

/**
 * Send a wake-up signal to a sync object from interrupt
 * @param sync      a sync object
 * @return          LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_thread_sync_signal_isr(lv_thread_sync_t * sync) {
  chSemSignalI(&sync->sem);
  return LV_RESULT_OK;
}

/**
 * Delete a sync object
 * @param sync      a sync object to delete
 * @return          LV_RESULT_OK: success; LV_RESULT_INVALID: failure
 */
lv_result_t lv_thread_sync_delete(lv_thread_sync_t * sync) {
  chSemObjectDispose(&sync->sem);
  return LV_RESULT_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /*LV_USE_OS == LV_OS_CHIBIOS*/
