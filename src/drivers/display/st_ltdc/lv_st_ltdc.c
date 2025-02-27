/**
 * @file lv_st_ltdc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "../../../lv_conf_internal.h"
#if LV_USE_ST_LTDC

#include "lv_st_ltdc.h"
#include "../../../display/lv_display_private.h"
#include "../../../draw/sw/lv_draw_sw.h"
#include "hal.h"
#include "hal_ltdc_lld.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef lv_thread_sync_t sync_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_display_t * create(void * buf1, void * buf2, uint32_t buf_size, uint32_t layer_idx,
                             lv_display_render_mode_t mode);
static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
static void flush_wait_cb(lv_display_t * disp);
static lv_color_format_t get_lv_cf_from_layer_cf(uint32_t cf);
static void reload_event_callback(LTDCDriver *ltdcp);

/**********************
 *  STATIC VARIABLES
 **********************/

static struct {
    bool disp_flushed_in_flush_cb[LTDC_MAX_LAYER];
    sync_t sync[LTDC_MAX_LAYER];
    volatile bool layer_interrupt_is_owned[LTDC_MAX_LAYER];
} g_data;

/*===========================================================================*/
/* LTDC related.                                                             */
/*===========================================================================*/

uint8_t frame_buffer[240 * 320 * 3] __attribute__((section(".ram7")));

static const ltdc_window_t ltdc_fullscreen_wincfg = {
  0,
  240 - 1,
  0,
  320 - 1,
};

static const ltdc_frame_t ltdc_screen_frmcfg1 = {
  frame_buffer,
  240,
  320,
  240 * 3,
  LTDC_FMT_RGB888,
};

static const ltdc_laycfg_t ltdc_screen_laycfg1 = {
  &ltdc_screen_frmcfg1,
  &ltdc_fullscreen_wincfg,
  LTDC_COLOR_WHITE,
  0xFF,
  0x980088,
  NULL,
  0,
  LTDC_BLEND_FIX1_FIX2,
  0,
};

static const LTDCConfig ltdc_cfg = {
  /* Display specifications.*/
  240,                              /**< Screen pixel width.*/
  320,                              /**< Screen pixel height.*/
  10,                               /**< Horizontal sync pixel width.*/
  2,                                /**< Vertical sync pixel height.*/
  20,                               /**< Horizontal back porch pixel width.*/
  2,                                /**< Vertical back porch pixel height.*/
  10,                               /**< Horizontal front porch pixel width.*/
  4,                                /**< Vertical front porch pixel height.*/
  0,                                /**< Driver configuration flags.*/

  /* ISR callbacks.*/
  NULL,                             /**< Line Interrupt ISR, or @p NULL.*/
  NULL,                             /**< Register Reload ISR, or @p NULL.*/
  NULL,                             /**< FIFO Underrun ISR, or @p NULL.*/
  NULL,                             /**< Transfer Error ISR, or @p NULL.*/

  /* Color and layer settings.*/
  LTDC_COLOR_WHITE,
  &ltdc_screen_laycfg1,
  NULL,
};

static const LTDCConfig ltdc_cfg_cb = {
  /* Display specifications.*/
  240,                              /**< Screen pixel width.*/
  320,                              /**< Screen pixel height.*/
  10,                               /**< Horizontal sync pixel width.*/
  2,                                /**< Vertical sync pixel height.*/
  20,                               /**< Horizontal back porch pixel width.*/
  2,                                /**< Vertical back porch pixel height.*/
  10,                               /**< Horizontal front porch pixel width.*/
  4,                                /**< Vertical front porch pixel height.*/
  0,                                /**< Driver configuration flags.*/

  /* ISR callbacks.*/
  NULL,                             /**< Line Interrupt ISR, or @p NULL.*/
  (ltdc_isrcb_t)reload_event_callback, /**< Register Reload ISR, or @p NULL.*/
  NULL,                             /**< FIFO Underrun ISR, or @p NULL.*/
  NULL,                             /**< Transfer Error ISR, or @p NULL.*/

  /* Color and layer settings.*/
  LTDC_COLOR_WHITE,
  &ltdc_screen_laycfg1,
  NULL,
};

/**********************
 *      MACROS
 **********************/

#define SYNC_INIT(layer_idx) lv_thread_sync_init(&g_data.sync[layer_idx])
#define SYNC_WAIT(layer_idx) lv_thread_sync_wait(&g_data.sync[layer_idx])
#define SYNC_SIGNAL_ISR(layer_idx) lv_thread_sync_signal_isr(&g_data.sync[layer_idx])

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_display_t * lv_st_ltdc_create_direct(void * fb_adr_1, void * fb_adr_2, uint32_t layer_idx)
{
    ltdcInit();

    if(fb_adr_1 != NULL && fb_adr_2 != NULL) {
        ltdcStart(&LTDCD1, &ltdc_cfg_cb);
    }
    else {
        ltdcStart(&LTDCD1, &ltdc_cfg);
    }

    ltdcShowLayer(&LTDCD1, 1, 1);
    return create(fb_adr_1, fb_adr_2, 0, layer_idx, LV_DISPLAY_RENDER_MODE_DIRECT);
}

lv_display_t * lv_st_ltdc_create_partial(void * render_buf_1, void * render_buf_2, uint32_t buf_size,
                                         uint32_t layer_idx)
{
    ltdcInit();
    ltdcStart(&LTDCD1, &ltdc_cfg);
    ltdcShowLayer(&LTDCD1, 1, 1);
    return create(render_buf_1, render_buf_2, buf_size, layer_idx, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_display_t * create(void * buf1, void * buf2, uint32_t buf_size, uint32_t layer_idx,
                             lv_display_render_mode_t mode)
{

    ltdc_frame_t * layer_cfg;
    if (layer_idx == 0) {
        layer_cfg = (ltdc_frame_t *)LTDCD1.config->lay1cfg->frame;
    }
    else {
        layer_cfg = (ltdc_frame_t *)LTDCD1.config->lay2cfg->frame;
    }

    uint32_t layer_width = layer_cfg->width;
    uint32_t layer_height = layer_cfg->height;
    uint32_t layer_cf = layer_cfg->fmt;
    lv_color_format_t cf = get_lv_cf_from_layer_cf(layer_cf);

    lv_display_t * disp = lv_display_create(layer_width, layer_height);
    lv_display_set_color_format(disp, cf);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_flush_wait_cb(disp, flush_wait_cb);
    lv_display_set_driver_data(disp, (void *)(uintptr_t)layer_idx);

    if(mode == LV_DISPLAY_RENDER_MODE_DIRECT) {
        uint32_t cf_size = lv_color_format_get_size(cf);
        lv_display_set_buffers(disp, buf1, buf2, layer_width * layer_height * cf_size, LV_DISPLAY_RENDER_MODE_DIRECT);

        if(buf1 != NULL && buf2 != NULL) {
            SYNC_INIT(layer_idx);
        }
    }
    else {
        lv_display_set_buffers(disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    }

    return disp;
}

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    uint32_t layer_idx = (uint32_t)(uintptr_t)lv_display_get_driver_data(disp);
    g_data.disp_flushed_in_flush_cb[layer_idx] = false;

    ltdc_frame_t * layer_cfg;
      if (layer_idx == 0) {
        layer_cfg = (ltdc_frame_t *)LTDCD1.config->lay1cfg->frame;
      }
      else {
        layer_cfg = (ltdc_frame_t *)LTDCD1.config->lay2cfg->frame;
      }

    if(disp->render_mode == LV_DISPLAY_RENDER_MODE_DIRECT) {
        if(lv_display_is_double_buffered(disp) && lv_display_flush_is_last(disp)) {
            layer_cfg->bufferp = px_map;
            g_data.layer_interrupt_is_owned[layer_idx] = true;
            ltdcReload(&LTDCD1, LTDC_RELOAD_VBR);
        }
        else {
            g_data.disp_flushed_in_flush_cb[layer_idx] = true;
        }
    }
    else {
        lv_color_format_t cf = lv_display_get_color_format(disp);
        int32_t disp_width = disp->hor_res;

        uint8_t * fb = (uint8_t *) layer_cfg->bufferp;
        uint32_t px_size = lv_color_format_get_size(cf);
        uint32_t fb_stride = px_size * disp_width;
        lv_area_t rotated_area = *area;
        lv_display_rotate_area(disp, &rotated_area);
        uint8_t * first_pixel = fb + fb_stride * rotated_area.y1 + px_size * rotated_area.x1;

        int32_t area_width = lv_area_get_width(area);
        int32_t area_height = lv_area_get_height(area);

        lv_display_rotation_t rotation = lv_display_get_rotation(disp);
        if(rotation == LV_DISPLAY_ROTATION_0) {
            uint32_t area_stride = px_size * area_width;
            uint8_t * fb_p = first_pixel;
            uint8_t * px_map_p = px_map;
            for(int i = 0; i < area_height; i++) {
                lv_memcpy(fb_p, px_map_p, area_stride);
                fb_p += fb_stride;
                px_map_p += area_stride;
            }
            g_data.disp_flushed_in_flush_cb[layer_idx] = true;
        }
        else {
            uint32_t area_stride = px_size * area_width;
            lv_draw_sw_rotate(px_map, first_pixel, area_width, area_height, area_stride, fb_stride, rotation, cf);
            g_data.disp_flushed_in_flush_cb[layer_idx] = true;
        }
    }
}

static void flush_wait_cb(lv_display_t * disp)
{
    uint32_t layer_idx = (uint32_t)(uintptr_t)lv_display_get_driver_data(disp);
    if(!g_data.disp_flushed_in_flush_cb[layer_idx]) {
        SYNC_WAIT(layer_idx);
    }
}

static lv_color_format_t get_lv_cf_from_layer_cf(uint32_t cf)
{
    switch(cf) {
        case LTDC_FMT_ARGB8888:
            return LV_COLOR_FORMAT_ARGB8888;
        case LTDC_FMT_RGB888:
            return LV_COLOR_FORMAT_RGB888;
        case LTDC_FMT_RGB565:
            return LV_COLOR_FORMAT_RGB565;
        case LTDC_FMT_L8:
            return LV_COLOR_FORMAT_L8;
        case LTDC_FMT_AL88:
            return LV_COLOR_FORMAT_AL88;
        default:
            LV_ASSERT_MSG(0, "the LTDC color format is not supported");
    }
}

static void reload_event_callback(LTDCDriver *ltdcp)
{
    (void)ltdcp;
    uint32_t i;
    for(i = 0; i < LTDC_MAX_LAYER; i++) {
        if(g_data.layer_interrupt_is_owned[i]) {
            g_data.layer_interrupt_is_owned[i] = false;
            SYNC_SIGNAL_ISR(i);
        }
    }
}

#endif /*LV_USE_ST_LTDC*/
