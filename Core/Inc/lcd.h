/**
 ****************************************************************************************************
 * @file        lcd.h
 * @author      ALIENTEK, recode by Chen Cheng
 * @version     V1.1
 * @date        2022-04-20
 * @brief       2.8寸/3.5寸/4.3寸/7寸 TFTLCD(MCU屏) 驱动代码头文件

 ****************************************************************************************************
 * @attention
 *
 * 
 ****************************************************************************************************
 */

#ifndef __LCD_H
#define __LCD_H

#include "stdlib.h"
#include "sys.h"


/******************************************************************************************/
/* LCD RST/WR/RD/BL/CS/RS 引脚定义
 * LCD_D0~D15,因数量太多，直接在lcd_init函数中修改
 * 在移植本函数时，除了修改头文件中的6个IO，还要修改lcd_init中的16个IO
 */

/* RESET 和系统复位Pin共用，不再重复定义 */
//#define LCD_RST_GPIO_PORT               GPIOx
//#define LCD_RST_GPIO_PIN                SYS_GPIO_PINx
//#define LCD_RST_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOx_CLK_ENABLE(); }while(0)   

#define LCD_WR_GPIO_PORT                GPIOD
#define LCD_WR_GPIO_PIN                 GPIO_PIN_5
#define LCD_WR_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)     

#define LCD_RD_GPIO_PORT                GPIOD
#define LCD_RD_GPIO_PIN                 GPIO_PIN_4
#define LCD_RD_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)     

#define LCD_BL_GPIO_PORT                GPIOB
#define LCD_BL_GPIO_PIN                 GPIO_PIN_5
#define LCD_BL_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     

/* LCD_CS和LCD_RS定义*/
/* 注意：这两个Pin分别需要根据LCD_FMC_NEX和LCD_FMC_AX设置正确的IO */
#define LCD_CS_GPIO_PORT                GPIOD
#define LCD_CS_GPIO_PIN                 GPIO_PIN_7
#define LCD_CS_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)     

#define LCD_RS_GPIO_PORT                GPIOD
#define LCD_RS_GPIO_PIN                 GPIO_PIN_13
#define LCD_RS_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)     

/* FMC相关参数
 * 注意：默认使用FMC Bank1，其共有4个片选: FMC_NE1~4
 *
 * 若修改LCD_FMC_NEX, 需同步修改LCD_CS_GPIO
 * 若修改LCD_FMC_AX , 需同步修改LCD_RS_GPIO
 */
#define LCD_FMC_NEX         1              /* 启用FMC_NE1连接LCD_CS时,取值范围只能为: 1~4 */
#define LCD_FMC_AX          18             /* 启用FMC_A18连接LCD_RS时,取值范围只能为: 0~25  */

/******************************************************************************************/

/* LCD参数 */
typedef struct
{
    uint16_t width;     /* LCD 宽度 */
    uint16_t height;    /* LCD 高度 */
    uint16_t id;        /* LCD ID */
    uint8_t dir;        /* 横屏或竖屏控制；1：横屏；0：竖屏 */
    uint16_t wramcmd;   /* gram开始写指令 */
    uint16_t setxcmd;   /* 设置x坐标指令 */
    uint16_t setycmd;   /* 设置y坐标指令 */
} _lcd_dev;

/* LCD参数 */
extern _lcd_dev lcddev; /* 管理LCD重要参数 */

/* LCD的画笔颜色和背景色 */
extern uint32_t  g_point_color;     /* 默认为红色 */
extern uint32_t  g_back_color;      /* 背景色，默认为白色 */

/* LCD背光控制 */
#define LCD_BL(x)   do{ x ? \
                      HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, GPIO_PIN_RESET); \
                     }while(0)

/* LCD地址结构体 */
typedef struct
{
    volatile uint16_t LCD_REG;
    volatile uint16_t LCD_RAM;
} LCD_TypeDef;


/* LCD_BASE的详细解算方法参见手册 */
#define LCD_BASE        (uint32_t)((0X60000000 + (0X4000000 * (LCD_FMC_NEX - 1))) | (((1 << LCD_FMC_AX) * 2) -2))
#define LCD             ((LCD_TypeDef *) LCD_BASE)

/******************************************************************************************/
/* LCD扫描方向和颜色 */

/* 扫描方向 */
#define L2R_U2D         0           
#define L2R_D2U         1           
#define R2L_U2D         2           
#define R2L_D2U         3           

#define U2D_L2R         4           
#define U2D_R2L         5           
#define D2U_L2R         6           
#define D2U_R2L         7           

#define DFT_SCAN_DIR    L2R_U2D     /* 默认 */

/* 常用画笔颜色 */
#define WHITE           0xFFFF      
#define BLACK           0x0000      
#define RED             0xF800      
#define GREEN           0x07E0      
#define BLUE            0x001F      
#define MAGENTA         0xF81F      /* 品红/紫红 = BLUE + RED */
#define YELLOW          0xFFE0      
#define CYAN            0x07FF      /* 青色 = GREEN + BLUE */  

/* 非常用颜色 */
#define BROWN           0xBC40      
#define BRRED           0xFC07      /* 棕红色 */
#define GRAY            0x8430      
#define DARKBLUE        0x01CF      
#define LIGHTBLUE       0x7D7C      
#define GRAYBLUE        0x5458      
#define LIGHTGREEN      0x841F       
#define LGRAY           0xC618      /* 浅灰色，窗体背景色 */ 
#define LGRAYBLUE       0xA651      /* 浅灰蓝色，中间层颜色 */ 
#define LBBLUE          0x2B12      /* 浅棕蓝色，选择条目的反色 */ 

/******************************************************************************************/
/* SSD1963配置，一般不需修改 */

#define SSD_HOR_RESOLUTION      800     
#define SSD_VER_RESOLUTION      480     

#define SSD_HOR_PULSE_WIDTH     1       
#define SSD_HOR_BACK_PORCH      46      
#define SSD_HOR_FRONT_PORCH     210     

#define SSD_VER_PULSE_WIDTH     1       
#define SSD_VER_BACK_PORCH      23      
#define SSD_VER_FRONT_PORCH     22      

/* 以下参数自动计算 */ 
#define SSD_HT          (SSD_HOR_RESOLUTION + SSD_HOR_BACK_PORCH + SSD_HOR_FRONT_PORCH)
#define SSD_HPS         (SSD_HOR_BACK_PORCH)
#define SSD_VT          (SSD_VER_RESOLUTION + SSD_VER_BACK_PORCH + SSD_VER_FRONT_PORCH)
#define SSD_VPS         (SSD_VER_BACK_PORCH)
   
/******************************************************************************************/
/* 函数声明 */

void lcd_wr_data(volatile uint16_t data);            
void lcd_wr_regno(volatile uint16_t regno);          
void lcd_write_reg(uint16_t regno, uint16_t data);   


void lcd_init(void);                                 
void lcd_display_on(void);                           
void lcd_display_off(void);                          
void lcd_scan_dir(uint8_t dir);                      
void lcd_display_dir(uint8_t dir);                   
void lcd_ssd_backlight_set(uint8_t pwm);             

void lcd_write_ram_prepare(void);                                                           
void lcd_set_cursor(uint16_t x, uint16_t y);                                               
uint32_t lcd_read_point(uint16_t x, uint16_t y);                                            
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color);                                

void lcd_clear(uint16_t color);                                                             
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                   
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);                  
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color);                 
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);             
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color);          
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);   
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);     
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);


void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color);
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color);
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color);
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color);


#endif

















