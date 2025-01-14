/**
 ****************************************************************************************************
 * @file        ltdc.c
 * @author      ALIENTEK，recode by Chen Cheng
 * @version     V1.0
 * @date        2022-4-20
 * @brief       LTDC 底层驱动
 ****************************************************************************************************
 * @attention
 *              默认竖屏显示，如需横屏在ltdc_init()中修改
 *              注意：若非通过CubeMX初始化，必须在stm32f4xx_hal_conf.h中启用ltdc module
 *
 ****************************************************************************************************
 */

#include "lcd.h"
#include "ltdc.h"
#include "stm32f4xx_hal_ltdc.h"         /* 不通过CubeMX启用FMC时必须引用ltdc驱动库头文件 */
#include "stm32f4xx_hal_dma2d.h"        /* 不通过CubeMX启用FMC时必须引用dma2d驱动库头文件 */

LTDC_HandleTypeDef  g_ltdc_handle;       /* LTDC句柄 */
DMA2D_HandleTypeDef g_dma2d_handle;      /* DMA2D句柄 */

#if !(__ARMCC_VERSION >= 6010050)                                                    /* 使用AC5编译器时 */


#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    uint32_t ltdc_lcd_framebuf[1280][800] __attribute__((at(LTDC_FRAME_BUF_ADDR)));   
#else
    #pragma location = LTDC_FRAME_BUF_ADDR          /* 注意：这种写法只在IAR EW中有效，使用MDK-ARM时参见手册 */
    uint16_t ltdc_lcd_framebuf[1280][800];          /* 注意：未正确转译该语句可能导致SDRAM无法访问！ */
//    uint16_t ltdc_lcd_framebuf1[1280][800] __attribute__((at(LTDC_FRAME_BUF_ADDR + 1280 * 800 * 2)));   
#endif

#else      /* 使用AC6编译器 */


#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    uint32_t ltdc_lcd_framebuf[1280][800] __attribute__((section(".bss.ARM.__at_0XC0000000")));  
#else
    uint16_t ltdc_lcd_framebuf[1280][800] __attribute__((section(".bss.ARM.__at_0XC0000000")));  
#endif

#endif

uint32_t *g_ltdc_framebuf[2];                /* LTDC LCD֡帧缓存数组指针 */
_ltdc_dev lcdltdc;                           /* LTDC参数 */

/**
 * @brief       LTDC开关
 * @param       sw   : 1 打开,0 关闭
 * @retval      无
 */
void ltdc_switch(uint8_t sw)
{
    if (sw)
    {
        __HAL_LTDC_ENABLE(&g_ltdc_handle);
    }
    else
    {
        __HAL_LTDC_DISABLE(&g_ltdc_handle);
    }
}

/**
 * @brief       LTDC开关指定层
 * @param       layerx       : 0,第一层; 1,第二层
 * @param       sw           : 1 打开;   0 关闭
 * @retval      无
 */
void ltdc_layer_switch(uint8_t layerx, uint8_t sw)
{
    if (sw) 
    {
        __HAL_LTDC_LAYER_ENABLE(&g_ltdc_handle, layerx);
    }
    else
    {
        __HAL_LTDC_LAYER_DISABLE(&g_ltdc_handle, layerx);
    }

    __HAL_LTDC_RELOAD_CONFIG(&g_ltdc_handle);
}

/**
 * @brief       LTDC选择层
 * @param       layerx   : 层号;0,第一层; 1,第二层;
 * @retval      无
 */
void ltdc_select_layer(uint8_t layerx)
{
    lcdltdc.activelayer = layerx;
}

/**
 * @brief       LTDC显示方向设置
 * @param       dir          : 0,竖屏 1,横屏
 * @retval      无
 */
void ltdc_display_dir(uint8_t dir)
{
    lcdltdc.dir = dir;    /* 显示方向 */

    if (dir == 0)         /* 竖屏 */
    {
        lcdltdc.width = lcdltdc.pheight;
        lcdltdc.height = lcdltdc.pwidth;
    }
    else if (dir == 1)    /* 横屏 */
    {
        lcdltdc.width = lcdltdc.pwidth;
        lcdltdc.height = lcdltdc.pheight;
    }
}

/**
 * @brief       LTDC画点
 * @param       x,y         : 写入坐标
 * @param       color       : 颜色值
 * @retval      无
 */
void ltdc_draw_point(uint16_t x, uint16_t y, uint32_t color)
{ 
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    if (lcdltdc.dir)   
    {
        *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x)) = color;
    }
    else               
    {
        *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)) = color; 
    }
#else
    if (lcdltdc.dir)   
    {
        *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x)) = color;
    }
    else              
    {
        *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)) = color; 
    }
#endif
}

/**
 * @brief       LTDC读点
 * @param       x,y       : 点坐标
 * @retval      颜色值
 */
uint32_t ltdc_read_point(uint16_t x, uint16_t y)
{ 
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    if (lcdltdc.dir)   
    {
        return *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x));
    }
    else               
    {
        return *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)); 
    }
#else
    if (lcdltdc.dir)   
    {
        return *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x));
    }
    else               
    {
        return *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)); 
    }
#endif 
}

/**
 * @brief       LTDC填充矩形（DMA2D填充）
 *  @note       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 *              注意:sx,ex,不能大于lcddev.width - 1; sy,ey,不能大于lcddev.height - 1, 否则非法
 * @param       sx,sy       : 起始坐标
 * @param       ex,ey       : 结束坐标
 * @param       color       : 待填充颜色
 * @retval      无
 */
void ltdc_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{ 
    uint32_t psx, psy, pex, pey;   
    uint32_t timeout = 0; 
    uint16_t offline;
    uint32_t addr; 

    
    if (lcdltdc.dir)               
    {
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    }
    else                         
    {
        if (ex >= lcdltdc.pheight)
        {
            ex = lcdltdc.pheight - 1;  
        }
        if (sx >= lcdltdc.pheight)
        {
            sx = lcdltdc.pheight - 1; 
        }

        psx = sy;
        psy = lcdltdc.pheight - ex - 1;
        pex = ey;
        pey = lcdltdc.pheight - sx - 1;
    }

    offline = lcdltdc.pwidth - (pex - psx + 1);
    addr = ((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));

    /* HAL库对于DMA2D的操作函数效率低下，故此处DMA2D全部使用寄存器直接操作 */
    __HAL_RCC_DMA2D_CLK_ENABLE();                              
    DMA2D->CR &= ~(DMA2D_CR_START);                            
    DMA2D->CR = DMA2D_R2M;                                     
    DMA2D->OPFCCR = LTDC_PIXFORMAT;                            
    DMA2D->OOR = offline;                                      
    DMA2D->OMAR = addr;                                       
    DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16);    
    DMA2D->OCOLR = color;                                      
    DMA2D->CR |= DMA2D_CR_START;                               

    while ((DMA2D->ISR & (DMA2D_FLAG_TC)) == 0)                
    {
        timeout++;
        if (timeout > 0X1FFFFF)break;                          
    } 
    DMA2D->IFCR |= DMA2D_FLAG_TC;                              
}

/**
 * @brief       指定区域内填充指定颜色块（DMA2D填充）
 * @note        仅支持uint16_t，RGB565格式
 *              (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 *              注意:sx,ex,不能大于lcddev.width - 1; sy,ey,不能大于lcddev.height - 1
 * @param       sx,sy       : 起始坐标
 * @param       ex,ey       : 结束坐标
 * @param       color       : 填充的颜色数组首地址
 * @retval      无
 */
void ltdc_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint32_t psx, psy, pex, pey;   
    uint32_t timeout = 0; 
    uint16_t offline;
    uint32_t addr;
 
    if (lcdltdc.dir)               
    {
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    }
    else                          
    {
        psx = sy;
        psy = lcdltdc.pheight - ex - 1;
        pex = ey;
        pey = lcdltdc.pheight - sx - 1;
    }
    
    offline = lcdltdc.pwidth - (pex - psx + 1);
    addr = ((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));

    RCC->AHB1ENR |= 1 << 23;            

    DMA2D->CR = 0 << 16;                
    DMA2D->FGPFCCR = LTDC_PIXFORMAT;    
    DMA2D->FGOR = 0;                    
    DMA2D->OOR = offline;               
    DMA2D->CR &= ~(1 << 0);             
    DMA2D->FGMAR = (uint32_t)color;     
    DMA2D->OMAR = addr;                 
    DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16); 
    DMA2D->CR |= 1 << 0;                

    while ((DMA2D->ISR & (1 << 1)) == 0)
    {
        timeout++;

        if (timeout > 0X1FFFFF)break;   
    }

    DMA2D->IFCR |= 1 << 1;              
}  

/**
 * @brief       LTCD清屏
 * @param       color          : 颜色值
 * @retval      无
 */
void ltdc_clear(uint32_t color)
{
    ltdc_fill(0, 0, lcdltdc.width - 1, lcdltdc.height - 1, color);
}

/**
 * @brief       LTDC时钟设置（Fdclk）
 * @param       pllsain     : SAI时钟倍频系数N,   取值范围:50~432.
 * @param       pllsair     : SAI时钟分频系数R，  取值范围:2~7
 * @param       pllsaidivr  : LCD时钟分频系数,    取值范围:0~3, 对应分频 2^(pllsaidivr + 1)
 *
 * @note        Fvco  = Fin * pllsain;
 *              Fdclk = Fvco / pllsair / 2 * 2^pllsaidivr = Fin * pllsain / pllsair / 2 * 2^pllsaidivr;
 *              其中:
 *              Fvco:VCO频率
 *              Fin:输入时钟频率，一般为1Mhz（来自sysclk PLLM分频后时钟，见时钟图）
 *              假设:外部高速晶振频率25Mhz, pllm = 25 时, Fin = 1Mhz.
 *              例如:要得到33M的LTDC时钟, 则可以设置: pllsain = 396, pllsair = 3, pllsaidivr = 1
 *              Fdclk= 1 * 396 / 3 / 2 * 2^1 = 396 / 12 = 33Mhz
 * @retval      0, 成功;
 *              其他, 失败;
 */
uint8_t ltdc_clk_set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr)
{
    RCC_PeriphCLKInitTypeDef periphclk_initure;

    periphclk_initure.PeriphClockSelection = RCC_PERIPHCLK_LTDC;     /* LTDC时钟 */
    periphclk_initure.PLLSAI.PLLSAIN = pllsain;
    periphclk_initure.PLLSAI.PLLSAIR = pllsair;
    periphclk_initure.PLLSAIDivR = pllsaidivr;

    if (HAL_RCCEx_PeriphCLKConfig(&periphclk_initure) == HAL_OK)     
    {
        return 0;                                                   
    }
    else
    {
        return 1;                                                   
    }
}

/**
 * @brief       LTDC层窗口设置, 窗口以LCD面板坐标为基准
 * @note        此函数必须在ltdc_layer_parameter_config之后设置。
 *              若窗口值不等于面板尺寸，GRAM操作（读/写点）也要根据窗口尺寸进行修改
 * @param       layerx      : 0,第一层; 1,第二层;
 * @param       sx, sy      : 起始坐标
 * @param       width,height: 宽度，高度
 * @retval      无
 */
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    HAL_LTDC_SetWindowPosition(&g_ltdc_handle, sx, sy, layerx);   /* 设置窗口位置 */
    HAL_LTDC_SetWindowSize(&g_ltdc_handle, width, height, layerx);/* 设置窗口大小 */
}

/**
 * @brief       LTDC层基本参数设置
 * @note        该设置必须在ltdc_layer_window_config前完成
 * @param       layerx      : 0,第一层; 1,第二层;
 * @param       bufaddr     : 层颜色帧缓存起始地址
 * @param       pixformat   : 颜色格式. 0,ARGB8888; 1,RGB888; 2,RGB565; 3,ARGB1555; 4,ARGB4444; 5,L8; 6;AL44; 7;AL88
 * @param       alpha       : 层颜色Alphaֵ, 0,全透明;255,不透明
 * @param       alpha0      : 默认颜色Alphaֵ, 0,全透明;255,不透明
 * @param       bfac1       : 混合系数1, 4(100),恒定的Alpha; 6(101),像素Alpha*恒定Alpha
 * @param       bfac2       : 混合系数2, 5(101),恒定的Alpha; 7(111),像素Alpha*恒定Alpha
 * @param       bkcolor     : 层默认颜色,32位,低24位有效,RGB888格式
 * @retval      无
 */
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor)
{
    LTDC_LayerCfgTypeDef playercfg;

    playercfg.WindowX0 = 0;                                            
    playercfg.WindowY0 = 0;                                            
    playercfg.WindowX1 = lcdltdc.pwidth;                               
    playercfg.WindowY1 = lcdltdc.pheight;                              
    playercfg.PixelFormat = pixformat;                                 
    playercfg.Alpha = alpha;                                           
    playercfg.Alpha0 = alpha0;                                         
    playercfg.BlendingFactor1 = (uint32_t)bfac1 << 8;                  
    playercfg.BlendingFactor2 = (uint32_t)bfac2;                       
    playercfg.FBStartAdress = bufaddr;                                 
    playercfg.ImageWidth = lcdltdc.pwidth;                             
    playercfg.ImageHeight = lcdltdc.pheight;                           
    playercfg.Backcolor.Red = (uint8_t)(bkcolor & 0X00FF0000) >> 16;   
    playercfg.Backcolor.Green = (uint8_t)(bkcolor & 0X0000FF00) >> 8;  
    playercfg.Backcolor.Blue = (uint8_t)bkcolor & 0X000000FF;          
    HAL_LTDC_ConfigLayer(&g_ltdc_handle, &playercfg, layerx);          
}  

/**
 * @brief       LTDC读取面板ID
 * @note        利用LCD RGB线的最高位(R7,G7,B7)来识别面板ID
 *              PG6 = R7(M0); PI2 = G7(M1); PI7 = B7(M2);
 *              M2:M1:M0
 *              0 :0 :0     4.3   480*272  RGB,ID = 0X4342
 *              0 :0 :1     7   800*480  RGB,ID = 0X7084
 *              0 :1 :0     7   1024*600 RGB,ID = 0X7016
 *              0 :1 :1     7   1280*800 RGB,ID = 0X7018
 *              1 :0 :0     4.3 800*480  RGB,ID = 0X4384
 *              1 :0 :1     10.1 1280*800 RGB,ID = 0X1018
 * @param       无
 * @retval      0, 非法; 
 *              其他, LCD ID
 */
uint16_t ltdc_panelid_read(void)
{
    uint8_t idx = 0;

    GPIO_InitTypeDef gpio_init_struct;

    __HAL_RCC_GPIOG_CLK_ENABLE();                           
    __HAL_RCC_GPIOI_CLK_ENABLE();                           

    gpio_init_struct.Pin = GPIO_PIN_6;                      /* PG6 */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;                
    gpio_init_struct.Pull = GPIO_PULLUP;                   
    gpio_init_struct.Speed = GPIO_SPEED_HIGH;               
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);                
    
    gpio_init_struct.Pin = GPIO_PIN_2 | GPIO_PIN_7;         /* PI2,7 */
    HAL_GPIO_Init(GPIOI, &gpio_init_struct);                

    idx = (uint8_t)HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_6);     
    idx|= (uint8_t)HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_2) << 1;
    idx|= (uint8_t)HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_7) << 2;

    switch (idx)
    {
        case 0 : 
            return 0X4342;                                  

        case 1 : 
            return 0X7084;                                  

        case 2 : 
            return 0X7016;                                  

        case 3 : 
            return 0X7018;                                  

        case 4 : 
            return 0X4384;                                  

        case 5 : 
            return 0X1018;                                  

        default : 
            return 0;
    }
}

/**
 * @brief       LTDC初始化
 * @param       无
 * @retval      无
 */
void ltdc_init(void)
{
    uint16_t ltdcid = 0;

    ltdcid = ltdc_panelid_read();                  /* 读取LCD面板ID */
    if (ltdcid == 0X4342)
    {
        lcdltdc.pwidth = 480;                     
        lcdltdc.pheight = 272;                    
        lcdltdc.hsw = 1;                          
        lcdltdc.vsw = 1;                          
        lcdltdc.hbp = 40;                         
        lcdltdc.vbp = 8;                          
        lcdltdc.hfp = 5;                          
        lcdltdc.vfp = 8;                          
        ltdc_clk_set(288, 4, RCC_PLLSAIDIVR_8);   
    }
    else if (ltdcid == 0X7084)
    {
        lcdltdc.pwidth = 800;                   
        lcdltdc.pheight = 480;                  
        lcdltdc.hsw = 1;                        
        lcdltdc.vsw = 1;                        
        lcdltdc.hbp = 46;                       
        lcdltdc.vbp = 23;                       
        lcdltdc.hfp = 210;                      
        lcdltdc.vfp = 22;                       
        ltdc_clk_set(396, 3, RCC_PLLSAIDIVR_4); 
    }
    else if (ltdcid == 0X7016)
    {
        lcdltdc.pwidth = 1024;                 
        lcdltdc.pheight = 600;                 
        lcdltdc.hsw = 20;                      
        lcdltdc.vsw = 3;                       
        lcdltdc.hbp = 140;                     
        lcdltdc.vbp = 20;                      
        lcdltdc.hfp = 160;                     
        lcdltdc.vfp = 12;                      
        ltdc_clk_set(360, 2, RCC_PLLSAIDIVR_4);
    }
    else if (ltdcid == 0X7018)
    {
        lcdltdc.pwidth = 1280;                  
        lcdltdc.pheight = 800;                  
    }
    else if (ltdcid == 0X4384)
    {
        lcdltdc.pwidth = 800;                   
        lcdltdc.pheight = 480;                  
        lcdltdc.hbp = 88;                       
        lcdltdc.hfp = 40;                       
        lcdltdc.hsw = 48;                       
        lcdltdc.vbp = 32;                       
        lcdltdc.vfp = 13;                       
        lcdltdc.vsw = 3;                        
        ltdc_clk_set(396, 3, RCC_PLLSAIDIVR_4); 
    }
    else if (ltdcid == 0X1018)                   
    {
        lcdltdc.pwidth = 1280;                  
        lcdltdc.pheight = 800;                  
        lcdltdc.hbp = 140;                     
        lcdltdc.hfp = 10;                       
        lcdltdc.hsw = 10;                       
        lcdltdc.vbp = 10;                       
        lcdltdc.vfp = 10;                      
        lcdltdc.vsw = 3;                        
        ltdc_clk_set(360, 2, RCC_PLLSAIDIVR_4); 
    } 

    lcddev.width = lcdltdc.pwidth;      
    lcddev.height = lcdltdc.pheight;    
    
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888 
    g_ltdc_framebuf[0] = (uint32_t*) &ltdc_lcd_framebuf;
    lcdltdc.pixsize = 4;                        
#else
    g_ltdc_framebuf[0] = (uint32_t*)&ltdc_lcd_framebuf;
    lcdltdc.pixsize = 2;                       
#endif 
    g_ltdc_handle.Instance = LTDC;
    g_ltdc_handle.Init.HSPolarity = LTDC_HSPOLARITY_AL;         
    g_ltdc_handle.Init.VSPolarity = LTDC_VSPOLARITY_AL;        
    g_ltdc_handle.Init.DEPolarity = LTDC_DEPOLARITY_AL;        
    g_ltdc_handle.State = HAL_LTDC_STATE_RESET;
    
    if (ltdcid == 0X1018)
    {
        g_ltdc_handle.Init.PCPolarity = LTDC_PCPOLARITY_IIPC;   
    }
    else 
    {
        g_ltdc_handle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;    
    }

    g_ltdc_handle.Init.HorizontalSync = lcdltdc.hsw - 1;                                            
    g_ltdc_handle.Init.VerticalSync = lcdltdc.vsw - 1;                                              
    g_ltdc_handle.Init.AccumulatedHBP = lcdltdc.hsw + lcdltdc.hbp - 1;                              
    g_ltdc_handle.Init.AccumulatedVBP = lcdltdc.vsw + lcdltdc.vbp - 1;                              
    g_ltdc_handle.Init.AccumulatedActiveW = lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth - 1;         
    g_ltdc_handle.Init.AccumulatedActiveH = lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight - 1;        
    g_ltdc_handle.Init.TotalWidth = lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth + lcdltdc.hfp - 1;   
    g_ltdc_handle.Init.TotalHeigh = lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight + lcdltdc.vfp - 1;  
    g_ltdc_handle.Init.Backcolor.Red = 0;                                                           
    g_ltdc_handle.Init.Backcolor.Green = 0;                                                         
    g_ltdc_handle.Init.Backcolor.Blue = 0;                                                          
    HAL_LTDC_Init(&g_ltdc_handle);

    ltdc_layer_parameter_config(0, (uint32_t)g_ltdc_framebuf[0], LTDC_PIXFORMAT, 255, 0, 6, 7, 0X000000);
    ltdc_layer_window_config(0, 0, 0, lcdltdc.pwidth, lcdltdc.pheight);                                  

    ltdc_display_dir(1);                /* 横屏或竖屏，默认竖屏 */
    ltdc_select_layer(0);                 /* 选择第一层 */
    LTDC_BL(1);                           /* 点亮背光 */
    ltdc_clear(0XFFFFFFFF);               /* 清屏 */
}

/**
 * @brief       LTDC底层IO初始化和时钟使能
 * @note        此函数会被HAL_LTDC_Init()调用
 * @param       hltdc:LTDC句柄
 * @retval      无
 */
void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    __HAL_RCC_LTDC_CLK_ENABLE();                      
    __HAL_RCC_DMA2D_CLK_ENABLE();                     

    LTDC_BL_GPIO_CLK_ENABLE();                        
    LTDC_DE_GPIO_CLK_ENABLE();                        
    LTDC_VSYNC_GPIO_CLK_ENABLE();                   
    LTDC_HSYNC_GPIO_CLK_ENABLE();                     
    LTDC_CLK_GPIO_CLK_ENABLE();                       
    __HAL_RCC_GPIOH_CLK_ENABLE();                     
    
    gpio_init_struct.Pin = LTDC_BL_GPIO_PIN;          
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;      
    gpio_init_struct.Pull = GPIO_PULLUP;              
    gpio_init_struct.Speed = GPIO_SPEED_HIGH;         
    HAL_GPIO_Init(LTDC_BL_GPIO_PORT, &gpio_init_struct);
    
    gpio_init_struct.Pin = LTDC_DE_GPIO_PIN;          
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;          
    gpio_init_struct.Alternate = GPIO_AF14_LTDC;      
    HAL_GPIO_Init(LTDC_DE_GPIO_PORT, &gpio_init_struct);
    
    gpio_init_struct.Pin = LTDC_VSYNC_GPIO_PIN;       
    HAL_GPIO_Init(LTDC_VSYNC_GPIO_PORT, &gpio_init_struct);
    
    gpio_init_struct.Pin = LTDC_HSYNC_GPIO_PIN;       
    HAL_GPIO_Init(LTDC_HSYNC_GPIO_PORT, &gpio_init_struct);
    
    gpio_init_struct.Pin = LTDC_CLK_GPIO_PIN;         
    HAL_GPIO_Init(LTDC_CLK_GPIO_PORT, &gpio_init_struct);

    /* PG6,11 */
    gpio_init_struct.Pin = GPIO_PIN_6 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);
    
    /* PH9,10,11,12,13,14,15 */
    gpio_init_struct.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
                     GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &gpio_init_struct);
    
    /* PI0,1,2,4,5,6,7 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5| \
                     GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOI, &gpio_init_struct); 
}

