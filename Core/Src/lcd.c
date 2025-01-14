/**
 ****************************************************************************************************
 * @file        lcd.c
 * @author      ALIENTEK, recode by Chen Cheng
 * @version     V1.2
 * @date        2024-04-20
 * @brief       2.8寸/3.5寸/4.3寸/7寸 TFTLCD(MCU屏) 驱动代码
 *             
 *
 ****************************************************************************************************
 * @attention
 *              注意：lcd及sram未在CubeMX中启用时，应手动在stm32f4xx_hal_conf.h中启用对应module！
 *
 *              此文件已经添加对RGB屏的支持
 ****************************************************************************************************
 */

#include <stdlib.h>
#include "lcd.h"
#include "lcdfont.h"
#include "usart.h"
#include "ltdc.h"
#include "stm32f4xx_hal_sram.h"     /* 不通过CubeMX启用FMC时必须引用sram驱动库头文件 */
//#include "stm32f4xx_hal.h"          /* 添加此行以包含SRAM_HandleTypeDef的定义 */

/* 注意：lcd_ex.c只用于LCD寄存器初始化，不要在其他的文件中include lcd_ex.c文件，也不要将该文件加入工程（或Cmake list）！ */
#include "lcd_ex.c"
//#include <stm32f4xx_ll_fmc.h>
//#include <stm32f4xx_ll_fsmc.h>


SRAM_HandleTypeDef g_sram_handle;       /* SRAM句柄 */

/* LCD画笔颜色和背景色 */
uint32_t g_point_color = 0xFF000000;    /* 画笔颜色 */
uint32_t g_back_color  = 0xFFFFFFFF;    /* 背景色 */

/* 管理LCD参数 */
_lcd_dev lcddev;

/**
 * @brief       LCD写数据
 * @param       data : 待写入数据
 * @retval      无
 */
void lcd_wr_data(volatile uint16_t data)
{
    data = data;            /* 插入延时 */
    LCD->LCD_RAM = data;
}

/**
 * @brief       LCD写寄存器编号/地址
 * @param       regno : 寄存器编号/地址
 * @retval      无
 */
void lcd_wr_regno(volatile uint16_t regno)
{
    regno = regno;          /* 插入延时 */
    LCD->LCD_REG = regno;   /* 写入寄存器序号 */
}

/**
 * @brief       LCD写寄存器
 * @param       regno : 寄存器编号/地址
 * @param       data : 待写入数据
 * @retval      无
 */
void lcd_write_reg(uint16_t regno, uint16_t data)
{
    LCD->LCD_REG = regno;   
    LCD->LCD_RAM = data;    
}

/**
 * @brief       LCD读数据
 * @param       无
 * @retval      读取到的数据
 */
static uint16_t lcd_rd_data(void)
{
    volatile uint16_t ram;  /* 防止编译器优化该参数 */

    ram = LCD->LCD_RAM;

    return ram;
}

/**
 * @brief       LCD延时函数
 * @param       i     : 延时数值
 * @retval      无
 */
static void lcd_opt_delay(uint32_t i)
{
    while (i--); 
}
  
/**
 * @brief       准备写GRAM
 * @param       无
 * @retval      无
 */
void lcd_write_ram_prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}

/**
 * @brief       读取某个点的颜色值
 * @param       x,y   : 点坐标
 * @retval      颜色（32位，兼容LTDC）
 */
uint32_t lcd_read_point(uint16_t x, uint16_t y)
{
    uint16_t r = 0, g = 0, b = 0;

    if (x >= lcddev.width || y >= lcddev.height)
    {
        return 0;                  /* 点超出范围则直接返回 */
    }

    if (lcdltdc.pwidth != 0)       /* 如果是RGB屏 */
    {
        return ltdc_read_point(x, y);
    }
    
    lcd_set_cursor(x, y);          /* 设置坐标 */

    if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(0X2E00);      /* 5510 发送读GRAM指令 */
    }
    else 
    {
        lcd_wr_regno(0X2E);        /* 9341/5310/1963/7789/7796/9806 发送读GRAM指令 */
    }

    r = lcd_rd_data();             /* dummy read */

    if (lcddev.id == 0x1963)
    {
        return r;                  /* 1963则直接读取 */
    }

    lcd_opt_delay(2);
    r = lcd_rd_data();             /* 实际坐标颜色 */
    
    if (lcddev.id == 0x7796)       /* 7796 一次读取一个像素值 */
    {
        return r;
    }
    
    /* 9341/5310/5510/7789/9806 分2次读出 */
    lcd_opt_delay(2);
    b = lcd_rd_data();
    g = r & 0xFF;                   /* 对于9341/5310/5510/7789/9806,第一次读取为RG值,R前G后,各8位 */
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));  /* ILI9341/NT35310/NT35510/ST7789/ILI9806需要公式转换 */
}

/**
 * @brief       LCD开启显示
 * @param       无
 * @retval      无
 */
void lcd_display_on(void)
{
    if (lcdltdc.pwidth != 0)    /* 如果接入RGB屏 */
    {
        ltdc_switch(1);         /* 开启LTDC */
    }
    else if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(0X2900); /* 开启显示 */
    }
    else     /* 9341/5310/1963/7789/7796/9806 等发送开启显示指令 */
    {
        lcd_wr_regno(0X29);   /* 开启显示 */
    }
}

/**
 * @brief       LCD关闭显示
 * @param       无
 * @retval      无
 */
void lcd_display_off(void)
{
    if (lcdltdc.pwidth != 0)    /* 如果接入RGB屏 */
    {
        ltdc_switch(0);         /* 关闭LTDC */
    }
    else if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(0X2800); /* 关闭显示 */
    }
    else    /* 9341/5310/1963/7789/7796/9806 等发送关闭显示指令 */
    {
        lcd_wr_regno(0X28);   /* 关闭显示 */
    }
}

/**
 * @brief       设置光标位置（不适用RGB屏）
 * @param       x,y   : 坐标
 * @retval      无
 */
void lcd_set_cursor(uint16_t x, uint16_t y)
{
    if (lcddev.id == 0x1963)
    {
        if (lcddev.dir == 0)    /* 竖屏模式下x坐标需要变换 */
        {
            x = lcddev.width - 1 - x;
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
        }
        else                    /* 横屏模式 */
        {
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
            lcd_wr_data((lcddev.width - 1) >> 8);
            lcd_wr_data((lcddev.width - 1) & 0xFF);
        }

        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0xFF);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(x & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(y & 0xFF);
    }
    else    /* 9341/5310/7789/7796/9806 设置坐标 */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_data(x & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0xFF);
    }
}

/**
 * @brief       设置LCD的自动扫描方向（不适用于RGB屏）
 * @note        9341/5310/5510/1963/7789/7796/9806等IC已经实际测试
 *              注意：其他函数可能受到此函数设置影响（尤其是9341）
 *              一般设置为L2R_U2D即可，其他扫描方式可能导致显示异常
 * @param       dir   : 0~7,代表8个方向，具体定义见lcd.h
 * @retval      无
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    if ((lcddev.dir == 1 && lcddev.id != 0x1963) || (lcddev.dir == 0 && lcddev.id == 0x1963))
    {
        switch (dir)   /* 方向转换 */
        {
            case 0:
                dir = 6;
                break;

            case 1:
                dir = 7;
                break;

            case 2:
                dir = 4;
                break;

            case 3:
                dir = 5;
                break;

            case 4:
                dir = 1;
                break;

            case 5:
                dir = 0;
                break;

            case 6:
                dir = 3;
                break;

            case 7:
                dir = 2;
                break;
        }
    }

    switch (dir)
    {
        case L2R_U2D:
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U:
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D:
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U:
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R:
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L:
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R:
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L:
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    dirreg = 0x36;  

    if (lcddev.id == 0x5510)
    {
        dirreg = 0x3600;    
    }

    if (lcddev.id == 0x9341 || lcddev.id == 0x7789 || lcddev.id == 0x7796)
    {
        regval |= 0x08;
    }

    lcd_write_reg(dirreg, regval);

    if (lcddev.id != 0x1963)                    
    {
        if (regval & 0x20)
        {
            if (lcddev.width < lcddev.height)   
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 2);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3);
        lcd_wr_data((lcddev.width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 2);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_regno(lcddev.setycmd + 3);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
    else
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0);
        lcd_wr_data(0);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_data((lcddev.width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0);
        lcd_wr_data(0);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
}

/**
 * @brief       画点
 * @param       x,y    : 坐标
 * @param       color  : 点颜色，32位
 * @retval      无
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    if (lcdltdc.pwidth != 0)
    {
        ltdc_draw_point(x, y, color);
    }
    else
    {
        lcd_set_cursor(x, y);     
        lcd_write_ram_prepare();  
        LCD->LCD_RAM = color; 
    }   
}

/**
 * @brief       SSD1963背光亮度设置
 * @param       pwm   : 背光等级，0~100
 * @retval      无
 */
void lcd_ssd_backlight_set(uint8_t pwm)
{
    lcd_wr_regno(0xBE);         
    lcd_wr_data(0x05);         
    lcd_wr_data(pwm * 2.55);    
    lcd_wr_data(0x01);          
    lcd_wr_data(0xFF);          
    lcd_wr_data(0x00);          
    lcd_wr_data(0x00);         
}

/**
 * @brief       设置LCD显示方向
 * @param       dir  : 0,竖屏; 1,横屏 
 * @retval      无
 */
void lcd_display_dir(uint8_t dir)
{
    lcddev.dir = dir;    

    if (lcdltdc.pwidth != 0)        /* 如果接入RGB屏 */
    {
        ltdc_display_dir(dir);
        lcddev.width = lcdltdc.width;
        lcddev.height = lcdltdc.height;
        return ;
    }       

    if (dir == 0)               
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0x2C00;
            lcddev.setxcmd = 0x2A00;
            lcddev.setycmd = 0x2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else if (lcddev.id == 0x1963)
        {
            lcddev.wramcmd = 0x2C;  
            lcddev.setxcmd = 0x2B;  
            lcddev.setycmd = 0x2A;  
            lcddev.width = 480;     
            lcddev.height = 800;    
        }
        else                        
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796)     
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }
        
        if (lcddev.id == 0X9806)    
        {
            lcddev.width = 480;
            lcddev.height = 800;
        }
    }                               
    else                            
    {
        lcddev.width = 320;         
        lcddev.height = 240;        

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0x2C00;
            lcddev.setxcmd = 0x2A00;
            lcddev.setycmd = 0x2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else if (lcddev.id == 0x1963 || lcddev.id == 0x9806)
        {
            lcddev.wramcmd = 0x2C;  
            lcddev.setxcmd = 0x2A;  
            lcddev.setycmd = 0x2B;  
            lcddev.width = 800;     
            lcddev.height = 480;    
        }
        else                       
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796)     
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    lcd_scan_dir(DFT_SCAN_DIR);     
}

/**
 * @brief       设置窗口（不适用于RGB屏），自动设置画点坐标至左上角(sx, sy)
 * @param       sx,sy:窗口起始坐标
 * @param       width,height:窗口宽度和高度，必须大于0
 * @note        窗体大小:width*height.
 * @retval      无
 */
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcdltdc.pwidth != 0)
    {
        return ;
    }

    if (lcddev.id == 0x1963 && lcddev.dir != 1)     
    {
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0xFF);
        lcd_wr_data((sx + width - 1) >> 8);
        lcd_wr_data((sx + width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0xFF);
        lcd_wr_data(height >> 8);
        lcd_wr_data(height & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(sx & 0xFF);
        lcd_wr_regno(lcddev.setxcmd + 2);
        lcd_wr_data(twidth >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3);
        lcd_wr_data(twidth & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(sy & 0xFF);
        lcd_wr_regno(lcddev.setycmd + 2);
        lcd_wr_data(theight >> 8);
        lcd_wr_regno(lcddev.setycmd + 3);
        lcd_wr_data(theight & 0xFF);
    }
    else    
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0xFF);
        lcd_wr_data(twidth >> 8);
        lcd_wr_data(twidth & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0xFF);
        lcd_wr_data(theight >> 8);
        lcd_wr_data(theight & 0xFF);
    }
}

/**
 * @brief       SRAM底层驱动
 * @note        此函数会被ᱻHAL_SRAM_Init()调用以初始化读写总线引脚
 * @param       hsram:SRAM句柄
 * @retval      无
 */
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
    GPIO_InitTypeDef gpio_init_struct;

    __HAL_RCC_FMC_CLK_ENABLE();                         
    __HAL_RCC_GPIOD_CLK_ENABLE();                       
    __HAL_RCC_GPIOE_CLK_ENABLE();                       

    /* PD0,1,4,5,7,8,9,10,13,14,15 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | \
                           GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13| GPIO_PIN_14 | GPIO_PIN_15;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;            
    gpio_init_struct.Pull = GPIO_PULLUP;                
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      
    gpio_init_struct.Alternate = GPIO_AF12_FMC;         

    HAL_GPIO_Init(GPIOD, &gpio_init_struct);           

    /* PE7,8,9,10,11,12,13,14,15 */
    gpio_init_struct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 \
                           | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
}

/**
 * @brief       初始化LCD
 * @param       无
 * @retval      无
 */
void lcd_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    FMC_NORSRAM_TimingTypeDef fmc_read_handle;
    FMC_NORSRAM_TimingTypeDef fmc_write_handle;

    lcddev.id = ltdc_panelid_read();
    if (lcddev.id != 0)
    {        /* 检测是否有RGB屏接入 */
        ltdc_init();
    }
    else{
        LCD_CS_GPIO_CLK_ENABLE();                               
        LCD_WR_GPIO_CLK_ENABLE();                               
        LCD_RD_GPIO_CLK_ENABLE();                               
        LCD_RS_GPIO_CLK_ENABLE();                               
        LCD_BL_GPIO_CLK_ENABLE();                               

        gpio_init_struct.Pin = LCD_CS_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                
        gpio_init_struct.Pull = GPIO_PULLUP;                    
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          
        gpio_init_struct.Alternate = GPIO_AF12_FMC;             
        HAL_GPIO_Init(LCD_CS_GPIO_PORT, &gpio_init_struct);     

        gpio_init_struct.Pin = LCD_WR_GPIO_PIN;
        HAL_GPIO_Init(LCD_WR_GPIO_PORT, &gpio_init_struct);     

        gpio_init_struct.Pin = LCD_RD_GPIO_PIN;
        HAL_GPIO_Init(LCD_RD_GPIO_PORT, &gpio_init_struct);     

        gpio_init_struct.Pin = LCD_RS_GPIO_PIN;
        HAL_GPIO_Init(LCD_RS_GPIO_PORT, &gpio_init_struct);     

        gpio_init_struct.Pin = LCD_BL_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            
        HAL_GPIO_Init(LCD_BL_GPIO_PORT, &gpio_init_struct);     

        g_sram_handle.Instance = FMC_NORSRAM_DEVICE;
        g_sram_handle.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
        
        g_sram_handle.Init.NSBank = FMC_NORSRAM_BANK1;                        
        g_sram_handle.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;     
        g_sram_handle.Init.MemoryType=FMC_MEMORY_TYPE_SRAM;                   
        g_sram_handle.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;    
        g_sram_handle.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;  
        g_sram_handle.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW; 
        g_sram_handle.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;      
        g_sram_handle.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;       
        g_sram_handle.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;              
        g_sram_handle.Init.ExtendedMode = FMC_EXTENDED_MODE_ENABLE;           
        g_sram_handle.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;  
        g_sram_handle.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;              
        g_sram_handle.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
        
        fmc_read_handle.AddressSetupTime = 0x0F;          
        fmc_read_handle.AddressHoldTime = 0x00;
        fmc_read_handle.DataSetupTime = 0x46;             

        fmc_read_handle.AccessMode = FMC_ACCESS_MODE_A;   
        fmc_write_handle.AddressSetupTime = 0x0F;         
        fmc_write_handle.AddressHoldTime = 0x00;
        fmc_write_handle.DataSetupTime = 0x0F;            
        fmc_write_handle.AccessMode = FMC_ACCESS_MODE_A;  
        HAL_SRAM_Init(&g_sram_handle, &fmc_read_handle, &fmc_write_handle);
        delay_ms(50);

        lcd_wr_regno(0xD3);
        lcddev.id = lcd_rd_data();          
        lcddev.id = lcd_rd_data();          
        lcddev.id = lcd_rd_data();          
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data();         

        if (lcddev.id != 0x9341)            
        {
            lcd_wr_regno(0x04);
            lcddev.id = lcd_rd_data();      
            lcddev.id = lcd_rd_data();      
            lcddev.id = lcd_rd_data();      
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data();     
            
            if (lcddev.id == 0x8552)        
            {
                lcddev.id = 0x7789;
            }

            if (lcddev.id != 0x7789)        
            {
                lcd_wr_regno(0xD4);
                lcddev.id = lcd_rd_data();  
                lcddev.id = lcd_rd_data();  
                lcddev.id = lcd_rd_data();  
                lcddev.id <<= 8;
                lcddev.id |= lcd_rd_data(); 

                if (lcddev.id != 0x5310)    
                {
                    lcd_wr_regno(0XD3);
                    lcddev.id = lcd_rd_data();  
                    lcddev.id = lcd_rd_data();  
                    lcddev.id = lcd_rd_data();  
                    lcddev.id <<= 8;
                    lcddev.id |= lcd_rd_data(); 
                    
                    if (lcddev.id != 0x7796)    
                    {
                        lcd_write_reg(0xF000, 0x0055);
                        lcd_write_reg(0xF001, 0x00AA);
                        lcd_write_reg(0xF002, 0x0052);
                        lcd_write_reg(0xF003, 0x0008);
                        lcd_write_reg(0xF004, 0x0001);
                        
                        lcd_wr_regno(0xC500);       
                        lcddev.id = lcd_rd_data();  
                        lcddev.id <<= 8;

                        lcd_wr_regno(0xC501);       
                        lcddev.id |= lcd_rd_data(); 
                        delay_ms(5);                


                        if (lcddev.id != 0x5510)    
                        {
                            lcd_wr_regno(0XD3);
                            lcddev.id = lcd_rd_data();  
                            lcddev.id = lcd_rd_data();  
                            lcddev.id = lcd_rd_data();  
                            lcddev.id <<= 8;
                            lcddev.id |= lcd_rd_data(); 
                            
                            if (lcddev.id != 0x9806)    
                            {
                                lcd_wr_regno(0xA1);
                                lcddev.id = lcd_rd_data();
                                lcddev.id = lcd_rd_data();  
                                lcddev.id <<= 8;
                                lcddev.id |= lcd_rd_data(); 

                                if (lcddev.id == 0x5761)lcddev.id = 0x1963; 
                            }
                        }
                    }
                }
            }
        }

        /* 注意：如果main函数中屏蔽了串口1的初始化，则程序会卡死在这个位置，因此必须初始化串口1，或屏蔽下方语句 */
        /* 如果不适用MDK开发，必须屏蔽下方语句，或确保IDE能够重定向printf至串口 */
        /*printf("LCD ID:%x\r\n", lcddev.id); */

        if (lcddev.id == 0x7789)
        {
            lcd_ex_st7789_reginit();        
        }
        else if (lcddev.id == 0x9341)
        {
            lcd_ex_ili9341_reginit();       
        }
        else if (lcddev.id == 0x5310)
        {
            lcd_ex_nt35310_reginit();       
        }
        else if (lcddev.id == 0x7796)
        {
            lcd_ex_st7796_reginit();        
        }
        else if (lcddev.id == 0x5510)
        {
            lcd_ex_nt35510_reginit();       
        }
        else if (lcddev.id == 0x9806)
        {
            lcd_ex_ili9806_reginit();  
        }
        else if (lcddev.id == 0x1963)
        {
            lcd_ex_ssd1963_reginit();       
            lcd_ssd_backlight_set(100);     
        }
    }

    /* 初始化完成后，提速 */
    if (lcddev.id == 0x9341 || lcddev.id == 0x7789)
    {
        fmc_write_handle.AddressSetupTime = 4;
        fmc_write_handle.DataSetupTime = 4;         
        FMC_NORSRAM_Extended_Timing_Init(g_sram_handle.Extended, &fmc_write_handle, g_sram_handle.Init.NSBank, g_sram_handle.Init.ExtendedMode);
    }
    else if (lcddev.id == 0x5310 || lcddev.id == 0x9806 || lcddev.id == 0x7796)
    {
        fmc_write_handle.AddressSetupTime = 3;
        fmc_write_handle.DataSetupTime = 3;
        FMC_NORSRAM_Extended_Timing_Init(g_sram_handle.Extended, &fmc_write_handle, g_sram_handle.Init.NSBank, g_sram_handle.Init.ExtendedMode);
    }
    else if (lcddev.id == 0x5510 || lcddev.id == 0x1963)
    {
        fmc_write_handle.AddressSetupTime = 2;
        fmc_write_handle.DataSetupTime = 2;
        FMC_NORSRAM_Extended_Timing_Init(g_sram_handle.Extended, &fmc_write_handle, g_sram_handle.Init.NSBank, g_sram_handle.Init.ExtendedMode);
    }

    lcd_display_dir(1);         /* 显示方向，默认为竖屏 */
    LCD_BL(1);          
    lcd_clear(WHITE);
}

/**
 * @brief       清屏
 * @param       color: 清屏颜色
 * @retval      无
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;

    if (lcdltdc.pwidth != 0)        /* 如果接入RGB屏 */
    {
        ltdc_clear(color);          /* LTDC清屏 */
    }
    else
    {
        totalpoint *= lcddev.height;    
        lcd_set_cursor(0x00, 0x0000);   
        lcd_write_ram_prepare();        

        for (index = 0; index < totalpoint; index++)
        {
            LCD->LCD_RAM = color;
        }
    }
}

/**
 * @brief       指定区域内填充颜色
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标；区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color:  待填充颜色，32位
 * @retval      无
 */
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{
    uint16_t i, j;
    uint16_t xlen = 0;

    if (lcdltdc.pwidth != 0)
    {
        ltdc_fill(sx, sy, ex, ey, color);
    }
    else
    {
        xlen = ex - sx + 1;
        for (i = sy; i <= ey; i++)
        {
            lcd_set_cursor(sx, i);      
            lcd_write_ram_prepare();    

            for (j = 0; j < xlen; j++)
            {
                LCD->LCD_RAM = color;   
            }
        }
    }
}

/**
 * @brief       指定区域内填充指定颜色块
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标；区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color: 待填充颜色，32位
 * @retval      无
 */
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;

    if (lcdltdc.pwidth != 0)        /* 如果接入RGB屏 */
    {
        ltdc_color_fill(sx, sy, ex, ey, color);
    }
    else
    {
        width = ex - sx + 1;                        
        height = ey - sy + 1;                       

        for (i = 0; i < height; i++)
        {
            lcd_set_cursor(sx, sy + i);             
            lcd_write_ram_prepare();                

            for (j = 0; j < width; j++)
            {
                LCD->LCD_RAM = color[i * width + j]; 
            }
        }
    }
}

/**
 * @brief       画线
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       color: 线的颜色
 * @retval      无
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    delta_x = x2 - x1;      
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1;           
    }
    else if (delta_x == 0)
    {
        incx = 0;           
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0;            
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y)
    {
        distance = delta_x;                 
    }
    else
    {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++ )    
    {
        lcd_draw_point(row, col, color);    
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * @brief       画水平线
 * @param       x,y   : 起点坐标
 * @param       len   : 线长度
 * @param       color : 矩形颜色
 * @retval      无
 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((len == 0) || (x > lcddev.width) || (y > lcddev.height))
    {
        return;
    }

    lcd_fill(x, y, x + len - 1, y, color);
}

/**
 * @brief       画矩形
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       color: 矩形颜色
 * @retval      无
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       画圆
 * @param       x0,y0 : 圆心坐标
 * @param       r     : 半径
 * @param       color : 画的颜色
 * @retval      无
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);       

    while (a <= b)
    {
        lcd_draw_point(x0 + a, y0 - b, color);  /* 5 */
        lcd_draw_point(x0 + b, y0 - a, color);  /* 0 */
        lcd_draw_point(x0 + b, y0 + a, color);  /* 4 */
        lcd_draw_point(x0 + a, y0 + b, color);  /* 6 */
        lcd_draw_point(x0 - a, y0 + b, color);  /* 1 */
        lcd_draw_point(x0 - b, y0 + a, color);
        lcd_draw_point(x0 - a, y0 - b, color);  /* 2 */
        lcd_draw_point(x0 - b, y0 - a, color);  /* 7 */
        a++;

        if (di < 0)
        {
            di += 4 * a + 6;
        }
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/**
 * @brief       填充实心圆
 * @param       x,y  : 圆心坐标
 * @param       r    : 半径
 * @param       color: 填充颜色
 * @retval      无
 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t xr = r;

    lcd_draw_hline(x - r, y, 2 * r, color);

    for (i = 1; i <= imax; i++)
    {
        if ((i * i + xr * xr) > sqmax)
        {
            /* draw lines from outside */
            if (xr > imax)
            {
                lcd_draw_hline (x - i + 1, y + xr, 2 * (i - 1), color);
                lcd_draw_hline (x - i + 1, y - xr, 2 * (i - 1), color);
            }

            xr--;
        }

        /* draw lines from inside (center) */
        lcd_draw_hline(x - xr, y + i, 2 * xr, color);
        lcd_draw_hline(x - xr, y - i, 2 * xr, color);
    }
}

/**
 * @brief       指定位置显示一个字符
 * @param       x,y  : 坐标
 * @param       chr  : 待显示的字符:" "--->"~"
 * @param       size : 字体大小 12/16/24/32
 * @param       mode : 叠加方式(1); 非叠加方式(0)
 * @param       color: 字体颜色
 * @retval      无
 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = 0;
    uint8_t *pfont = 0;

    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); 
    chr = chr - ' ';    

    switch (size)
    {
        case 12:
            pfont = (uint8_t *)asc2_1206[chr];  
            break;

        case 16:
            pfont = (uint8_t *)asc2_1608[chr];  
            break;

        case 24:
            pfont = (uint8_t *)asc2_2412[chr];  
            break;

        case 32:
            pfont = (uint8_t *)asc2_3216[chr];  
            break;

        default:
            return ;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t];                            

        for (t1 = 0; t1 < 8; t1++)                  
        {
            if (temp & 0x80)                        
            {
                lcd_draw_point(x, y, color);        
            }
            else if (mode == 0)                     
            {
                lcd_draw_point(x, y, g_back_color); 
            }

            temp <<= 1;                             
            y++;

            if (y >= lcddev.height)return;         

            if ((y - y0) == size)                   
            {
                y = y0;                             
                x++;                                

                if (x >= lcddev.width)
                {
                    return;                         
                }

                break;
            }
        }
    }
}

/**
 * @brief       平方函数, m^n
 * @param       m: 底数
 * @param       n: ָ指数
 * @retval      m的n次方
 */
static uint32_t lcd_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)
    {
        result *= m;
    }

    return result;
}

/**
 * @brief       显示len个数字
 * @param       x,y : 起始坐标
 * @param       num : 数值(0 ~ 2^32)
 * @param       len : 显示数字的位数
 * @param       size: 选择字体 12/16/24/32
 * @param       color: 字体颜色
 * @retval      无
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)                                               
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;                       

        if (enshow == 0 && t < (len - 1))                                   
        {
            if (temp == 0)
            {
                lcd_show_char(x + (size / 2) * t, y, ' ', size, 0, color);  
                continue;                                                   
            }
            else
            {
                enshow = 1;                                                 
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, 0, color);   
    }
}

/**
 * @brief       扩展显示len个数字（若高位是0也显示）
 * @param       x,y : 起始坐标
 * @param       num : 数值(0 ~ 2^32)
 * @param       len : 显示数字的位数
 * @param       size: 选择字体 12/16/24/32
 * @param       mode: 显示模式
 *              [7]:0,不填充;1,填充.
 *              [6:1]:保留
 *              [0]:0,非叠加;1,叠加.
 * @param       color: 字体颜色
 * @retval      无
 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)                                                             
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;                                     

        if (enshow == 0 && t < (len - 1))                                                 
        {
            if (temp == 0)
            {
                if (mode & 0x80)                                                          
                {
                    lcd_show_char(x + (size / 2) * t, y, '0', size, mode & 0x01, color);  
                }
                else
                {
                    lcd_show_char(x + (size / 2) * t, y, ' ', size, mode & 0x01, color);  
                }

                continue;
            }
            else
            {
                enshow = 1;                                                               
            }

        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, mode & 0x01, color);
    }
}

/**
 * @brief       显示字符串
 * @param       x,y         : 起始坐标
 * @param       width,height: 区域大小
 * @param       size        : 字体大小 12/16/24/32
 * @param       p           : 字符串首地址
 * @param       color       : 字体颜色
 * @retval      无
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint8_t x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' '))   
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)
        {
            break;                       
        }

        lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}















