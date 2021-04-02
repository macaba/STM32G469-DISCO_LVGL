/**
 * @file tft.h
 *
 */

#ifndef DISP_H
#define DISP_H

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include "lvgl/src/lv_misc/lv_color.h"
#include "lvgl/src/lv_misc/lv_area.h"

#define TFT_HOR_RES 800
#define TFT_VER_RES 480

void tft_init(void);

#endif
