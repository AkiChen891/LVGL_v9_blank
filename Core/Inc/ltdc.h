/**
 ****************************************************************************************************
 * @file        ltdc.h
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-4-20
 * @brief       LTDC 底层驱动 头文件
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */

#ifndef _LTDC_H
#define _LTDC_H

#include "sys.h"


/* LCD LTDC参数 */
typedef struct  
{
    uint32_t pwidth;      /* LTDC面板宽度，固定参数，不随显示方向改变。如果为0，说明没有RGB屏接入 */
    uint32_t pheight;     /* LTDC面板高度，固定参数，不随显示方向改变 */
    uint16_t hsw;         /* 水平同步宽度 */
    uint16_t vsw;         /* 垂直同步宽度 */
    uint16_t hbp;         /* 水平后廊 */
    uint16_t vbp;         /* 垂直后廊 */
    uint16_t hfp;         /* 水平前廊 */
    uint16_t vfp;         /* 垂直前廊  */
    uint8_t activelayer;  /* 当前层编号:0/1 */
    uint8_t dir;          /* 0,竖屏;1,横屏; */
    uint16_t width;       /* LTDC宽度 */
    uint16_t height;      /* LTDC高度 */
    uint32_t pixsize;     /* 每个像素所占字节数 */
}_ltdc_dev; 

extern _ltdc_dev lcdltdc;                   
extern LTDC_HandleTypeDef g_ltdc_handle;    
extern DMA2D_HandleTypeDef g_dma2d_handle;  

#define LTDC_PIXFORMAT_ARGB8888      0X00    /* ARGB8888 */
#define LTDC_PIXFORMAT_RGB888        0X01    /* RGB888 */
#define LTDC_PIXFORMAT_RGB565        0X02    /* RGB565 */
#define LTDC_PIXFORMAT_ARGB1555      0X03    /* ARGB1555 */
#define LTDC_PIXFORMAT_ARGB4444      0X04    /* ARGB4444 */
#define LTDC_PIXFORMAT_L8            0X05    /* L8 */
#define LTDC_PIXFORMAT_AL44          0X06    /* AL44 */
#define LTDC_PIXFORMAT_AL88          0X07    /* AL88 */

/******************************************************************************************/
/* LTDC_BL/DE/VSYNC/HSYNC/CLK 引脚定义 
 * LTDC_R3~R7, G2~G7, B3~B7，直接在lcd_init()中修改
 * 移植时除了修改以下5个IO，还需修改R3~R7, G2~G7, B3~B7
 */

/* BL和MCU屏背光共用，不再重复定义 */
#define LTDC_BL_GPIO_PORT               GPIOB
#define LTDC_BL_GPIO_PIN                GPIO_PIN_5
#define LTDC_BL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)    

#define LTDC_DE_GPIO_PORT               GPIOF
#define LTDC_DE_GPIO_PIN                GPIO_PIN_10
#define LTDC_DE_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)    

#define LTDC_VSYNC_GPIO_PORT            GPIOI
#define LTDC_VSYNC_GPIO_PIN             GPIO_PIN_9
#define LTDC_VSYNC_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOI_CLK_ENABLE(); }while(0)    

#define LTDC_HSYNC_GPIO_PORT            GPIOI
#define LTDC_HSYNC_GPIO_PIN             GPIO_PIN_10
#define LTDC_HSYNC_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOI_CLK_ENABLE(); }while(0)    

#define LTDC_CLK_GPIO_PORT              GPIOG
#define LTDC_CLK_GPIO_PIN               GPIO_PIN_7
#define LTDC_CLK_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOG_CLK_ENABLE(); }while(0)    


#define LTDC_PIXFORMAT                  LTDC_PIXFORMAT_RGB565

#define LTDC_BACKLAYERCOLOR             0X00000000

#define LTDC_FRAME_BUF_ADDR             0XC0000000

#define LTDC_BL(x)   do{ x ? \
                      HAL_GPIO_WritePin(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN, GPIO_PIN_RESET); \
                     }while(0)

/******************************************************************************************/

void ltdc_switch(uint8_t sw);                                                                                                                                         
void ltdc_layer_switch(uint8_t layerx, uint8_t sw);                                                                                                                   
void ltdc_select_layer(uint8_t layerx);                                                                                                                             
void ltdc_display_dir(uint8_t dir);                                                                                                                                   
void ltdc_draw_point(uint16_t x, uint16_t y, uint32_t color);                                                                                                      
uint32_t ltdc_read_point(uint16_t x, uint16_t y);                                                                                                                    
void ltdc_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color);                                                                                 
void ltdc_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);                                                                           
void ltdc_clear(uint32_t color);                                                                                                                                     
uint8_t ltdc_clk_set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr);                                                                                        
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);                                                             
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor); 
uint16_t ltdc_panelid_read(void);                                                                                                                                     
void ltdc_init(void);                                                                                                                                                


#endif 
