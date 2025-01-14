/**
 ****************************************************************************************************
 * @file        sys.h
 * @author      ALIENTEK, recode by Chen Cheng
 * @version     V1.0
 * @date        2022-4-20
 * @brief       系统初始化头文件
 * @license     
 ****************************************************************************************************
 * @attention
 *              不要改动此文件中的任何内容！！

 ****************************************************************************************************
 */

#ifndef _SYS_H
#define _SYS_H

#include "stm32f4xx.h"
#include "core_cm4.h"
#include "stm32f4xx_hal.h"
#include "main.h"


/**
 * SYS_SUPPORT_OS用于定义系统文件夹是否支持OS
 * 0,不支持OS
 * 1,支持OS
 */
#define SYS_SUPPORT_OS         0        /* 默认为不支持OS */


/*函数声明*******************************************************************************************/

void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset);                         /* 设置中断偏移量 */
void sys_standby(void);                                                                     /* 进入待机模式 */
void sys_soft_reset(void);                                                                  /* 系统软复位 */
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);   /* 配置系统时钟 */


/* 汇编函数 */
void sys_wfi_set(void);             /* 执行WFI指令 */
void sys_intx_disable(void);        /* 关闭所有中断 */
void sys_intx_enable(void);         /* 打开所有中断 */
void sys_msr_msp(uint32_t addr);    /* 设置栈顶地址 */

#endif

