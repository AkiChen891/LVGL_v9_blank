/**
 ****************************************************************************************************
 * @file        sdram.c
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-4-20
 * @brief       SDRAM 驱动文件
 ****************************************************************************************************
 * @attention
 *
 *              注意：若非通过CubeMX初始化，必须在stm32f4xx_hal_conf.h中启用sdram module
 ****************************************************************************************************
 */

#include "sdram.h"
#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "stm32f4xx_hal_sdram.h"


SDRAM_HandleTypeDef g_sdram_handle;       

/**
 * @brief       SDRAM初始化
 * @param       无
 * @retval      无
 */
void sdram_init(void)
{
    FMC_SDRAM_TimingTypeDef sdram_timing;

    g_sdram_handle.Instance = FMC_SDRAM_DEVICE;                              /* SDRAM位于BANK5,6 */
    g_sdram_handle.Init.SDBank = FMC_SDRAM_BANK1;                            /* 第一个SDRAM BANK */
    g_sdram_handle.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;      /* 列数量 */
    g_sdram_handle.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;           /* 行数量 */
    g_sdram_handle.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;        /* 数据宽度为16位 */
    g_sdram_handle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;   /* 一共4个BANK */
    g_sdram_handle.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;                /* CAS为3 */
    g_sdram_handle.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;/* 失能写保护 */
    g_sdram_handle.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;            /* SDRAM时钟=HCLK/2=192M/2=96M=10.4ns */
    g_sdram_handle.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;                 
    g_sdram_handle.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;             /* 读通道延时 */
    
    sdram_timing.LoadToActiveDelay = 2;                                      /* 加载模式寄存器到激活时间延迟 */
    sdram_timing.ExitSelfRefreshDelay = 7;                                   /* 退出自刷新延迟 */
    sdram_timing.SelfRefreshTime = 6;                                        /* 自刷新延迟 */
    sdram_timing.RowCycleDelay = 6;                                          /* 行循环延迟 */
    sdram_timing.WriteRecoveryTime = 2;                                      /* 恢复延迟 */
    sdram_timing.RPDelay = 2;                                                /* 行预充电延迟 */
    sdram_timing.RCDDelay = 2;                                               /* 行到列延迟 */
    HAL_SDRAM_Init(&g_sdram_handle, &sdram_timing);

    sdram_initialization_sequence();                                         /* 发送SDRAM初始化序列 */

    HAL_SDRAM_ProgramRefreshRate(&g_sdram_handle, 730);                      /* 设置刷新频率 */
}

/**
 * @brief       发送SDRAM初始化序列
 * @param       无
 * @retval      无
 */
void sdram_initialization_sequence(void)
{
    uint32_t temp = 0;

    sdram_send_cmd(0, FMC_SDRAM_CMD_CLK_ENABLE, 1, 0);             
    delay_us(500);                                                   
    sdram_send_cmd(0, FMC_SDRAM_CMD_PALL, 1, 0);                    
    sdram_send_cmd(0, FMC_SDRAM_CMD_AUTOREFRESH_MODE, 8, 0);        

    temp = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1  |                  
              SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL  |                  
              SDRAM_MODEREG_CAS_LATENCY_3          |                  
              SDRAM_MODEREG_OPERATING_MODE_STANDARD|                  
              SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;                   
    sdram_send_cmd(0, FMC_SDRAM_CMD_LOAD_MODE, 1, temp);              
}

/**
 * @brief       SDRAM底层驱动
 * @note        HAL_SDRAM_Init()会调用此函数
 * @param       hsdram:SDRAM句柄
 * @retval      
 */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    GPIO_InitTypeDef gpio_init_struct;

    __HAL_RCC_FMC_CLK_ENABLE();                  
    __HAL_RCC_GPIOC_CLK_ENABLE();               
    __HAL_RCC_GPIOD_CLK_ENABLE();                
    __HAL_RCC_GPIOE_CLK_ENABLE();                
    __HAL_RCC_GPIOF_CLK_ENABLE();                
    __HAL_RCC_GPIOG_CLK_ENABLE();              
    
    gpio_init_struct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;  
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;    
    gpio_init_struct.Pull = GPIO_PULLUP;         
    gpio_init_struct.Speed = GPIO_SPEED_HIGH;    
    gpio_init_struct.Alternate = GPIO_AF12_FMC;  
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);     
    
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &gpio_init_struct);     /* PD0,1,8,9,10,14,15 */
    
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                           GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);     /* PE0,1,7,8,9,10,11,12,13,14,15 */
    
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                           GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio_init_struct);     /* PF0,1,2,3,4,5,11,12,13,14,15 */
    
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);     /* PG0,1,2,4,5,8,15 */
}

/**
 * @brief       向SDRAM发送命令
 * @param       bankx:0,向BANK5 SDRAM发送指令
 * @param             1,向BANK6 SDRAM发送指令
 * @param       cmd:ָ指令(0,正常模式/1,时钟配置使能/2,预充电所有存储区/3,自动刷新/4,加载模式寄存器/5,自刷新/6,掉电)
 * @param       refresh:自刷新次数
 * @param       返回值:0,正常;1,失败.
 * @retval      模式寄存器的定义
 */
uint8_t sdram_send_cmd(uint8_t bankx, uint8_t cmd, uint8_t refresh, uint16_t regval)
{
    uint32_t target_bank = 0;
    FMC_SDRAM_CommandTypeDef command;
    
    if (bankx == 0)
    {
        target_bank = FMC_SDRAM_CMD_TARGET_BANK1;
    }
    else if (bankx == 1)
    {
        target_bank = FMC_SDRAM_CMD_TARGET_BANK2;
    }

    command.CommandMode = cmd;                                            
    command.CommandTarget = target_bank;                                    
    command.AutoRefreshNumber = refresh;                                    
    command.ModeRegisterDefinition = regval;                                

    if (HAL_SDRAM_SendCommand(&g_sdram_handle, &command, 0X1000) == HAL_OK) 
    {
        return 0;
    }
    else 
    {
        return 1;
    }
}

/**
 * @brief       在指定地址(addr+BANK5_SDRAM_ADDR)开始,写入n个字节
 * @param       pbuf  : 字节缓冲区指针
 * @param       addr  : 地址
 * @param       n     : 字节数
 * @retval      无
*/
void fmc_sdram_write_buffer(uint8_t *pbuf, uint32_t addr, uint32_t n)
{
    for (; n != 0; n--)
    {
        *(volatile uint8_t*)(BANK5_SDRAM_ADDR + addr) = *pbuf;
        addr++;
        pbuf++;
    }
}

/**
 * @brief       从指定地址((addr+BANK5_SDRAM_ADDR))开始,连续读出n个字节
 * @param       pbuf  : 字节缓冲区指针
 * @param       addr  : 起始地址
 * @param       n     : 字节个数
 * @retval      无
*/
void fmc_sdram_read_buffer(uint8_t *pbuf, uint32_t addr, uint32_t n)
{
    for (; n != 0; n--)
    {
        *pbuf++ = *(volatile uint8_t*)(BANK5_SDRAM_ADDR + addr);
        addr++;
    }
}

