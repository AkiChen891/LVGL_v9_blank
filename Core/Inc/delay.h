/**
 ****************************************************************************************************
 * @file        delay.h
 * @author      ALIENTEK, recode by Chen Cheng
 * @version     V1.1
 * @date        2023-02-28
 * @brief       系统延时头文件
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "sys.h"


void delay_init(uint16_t sysclk);           /* 延时初始化 */
void delay_ms(uint16_t nms);                /* 延时（毫秒） */
void delay_us(uint32_t nus);                /* 延时（微妙） */

void HAL_Delay(uint32_t Delay);             /* HAL库默认延时函数 */

#endif

