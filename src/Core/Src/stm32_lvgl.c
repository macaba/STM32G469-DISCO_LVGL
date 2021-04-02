#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include <string.h>
#include <stdlib.h>

#include "stm32_lvgl.h"
#include "stm32469i_discovery_lcd.h"

#if LV_COLOR_DEPTH != 16 && LV_COLOR_DEPTH != 24 && LV_COLOR_DEPTH != 32
#error LV_COLOR_DEPTH must be 16, 24, or 32
#endif

/*These 3 functions are needed by LittlevGL*/
static void ex_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p);
#if LV_USE_GPU
static void gpu_mem_blend(lv_disp_drv_t *disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
//Note: us LL_ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
static void gpu_mem_fill(lv_disp_drv_t *disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width, const lv_area_t * fill_area, lv_color_t color);
//Note: use LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex)
#endif

static lv_disp_t *our_disp = NULL;

void tft_init(void)
{
	/* There is only one display on STM32 */
	if(our_disp != NULL)
		abort();

   /*-----------------------------
	* Create a buffer for drawing
	*----------------------------*/

   /* LittlevGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row*/

	static lv_disp_buf_t disp_buf_1;
	static lv_color_t buf1_1[TFT_HOR_RES * 60];
	//static lv_color_t buf1_2[LV_HOR_RES_MAX * 68];
	lv_disp_buf_init(&disp_buf_1, buf1_1, NULL, TFT_HOR_RES * 60);   /*Initialize the display buffer*/


	/*-----------------------------------
	* Register the display in LittlevGL
	*----------------------------------*/

	lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

	/*Set up the functions to access to your display*/

	/*Set the resolution of the display*/
	disp_drv.hor_res = TFT_HOR_RES;
	disp_drv.ver_res = TFT_VER_RES;

	/*Used to copy the buffer's content to the display*/
	disp_drv.flush_cb = ex_disp_flush;

	/*Set a display buffer*/
	disp_drv.buffer = &disp_buf_1;


	/*Finally register the driver*/
	our_disp = lv_disp_drv_register(&disp_drv);
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p)
{
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
        	// BSP_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
        	BSP_LCD_DrawPixel(x, y, lv_color_to32(*color_p));  /* Put a pixel to the display.*/
        	//BSP_LCD_DrawPixel(x, y, 0);  /* Put a pixel to the display.*/
            color_p++;
        }
    }

    lv_disp_flush_ready(drv);         /* Indicate you are ready with the flushing*/
}

///**
// * Copy pixels to destination memory using opacity
// * @param dest a memory address. Copy 'src' here.
// * @param src pointer to pixel map. Copy it to 'dest'.
// * @param length number of pixels in 'src'
// * @param opa opacity (0, OPA_TRANSP: transparent ... 255, OPA_COVER, fully cover)
// */
//static void gpu_mem_blend(lv_disp_drv_t * drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
//{
//	/*Wait for the previous operation*/
//	HAL_DMA2D_PollForTransfer(&Dma2dHandle, 100);
//	Dma2dHandle.Init.Mode         = DMA2D_M2M_BLEND;
//	/* DMA2D Initialization */
//	if(HAL_DMA2D_Init(&Dma2dHandle) != HAL_OK)
//	{
//		/* Initialization Error */
//		while(1);
//	}
//
//	Dma2dHandle.LayerCfg[1].InputAlpha = opa;
//    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
//	HAL_DMA2D_BlendingStart(&Dma2dHandle, (uint32_t) src, (uint32_t) dest, (uint32_t)dest, length, 1);
//}
//
//static void gpu_mem_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color)
//{
//	/*Wait for the previous operation*/
//	HAL_DMA2D_PollForTransfer(&Dma2dHandle, 100);
//
//   Dma2dHandle.Init.Mode         = DMA2D_R2M;
//   /* DMA2D Initialization */
//   if(HAL_DMA2D_Init(&Dma2dHandle) != HAL_OK)
//   {
//     /* Initialization Error */
//     while(1);
//   }
//
//   Dma2dHandle.LayerCfg[1].InputAlpha = 0xff;
//   HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
//
//   lv_color_t * dest_buf_ofs = dest_buf;
//
//   dest_buf_ofs += dest_width * fill_area->y1;
//   dest_buf_ofs += fill_area->x1;
//   lv_coord_t area_w = lv_area_get_width(fill_area);
//
//   uint32_t i;
//   for(i = fill_area->y1; i <= fill_area->y2; i++) {
//	   /*Wait for the previous operation*/
//	   HAL_DMA2D_PollForTransfer(&Dma2dHandle, 100);
//	   HAL_DMA2D_BlendingStart(&Dma2dHandle, (uint32_t) lv_color_to32(color), (uint32_t) dest_buf_ofs, (uint32_t)dest_buf_ofs, area_w, 1);
//	   dest_buf_ofs += dest_width;
//   }
//}
