/**
  ******************************************************************************
  * @file           : led.c
  * @brief          : LED函数，包括初始化、点亮和翻转
  ******************************************************************************
  * @attention
  *
  *     此文件应当与main.c一同编译
  *     注意：LED占用PB0和PB1两个IO，CubeMX中若重复定义应保证参数完全一致
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include "main.h"

/* LED1（绿色）引脚定义为PB0 */
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
/* LED0（红色）引脚定义为PB1 */
#define LED0_Pin GPIO_PIN_1
#define LED0_GPIO_Port GPIOB

void led_init();
void led_green(uint8_t flag);
void led_red(uint8_t flag);
void led_red_toggle();
void led_green_toggle();

/** 
*   @brief LED初始化函数
*   @param NULL
*   @return NULL
*/
void led_init()
{
  /* 声明GPIO结构体 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 启用时钟 */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* 配置PB0和PB1为高电平 */
  HAL_GPIO_WritePin(GPIOB, LED1_Pin|LED0_Pin, GPIO_PIN_SET);

  /* 初始化GPIO */
  GPIO_InitStruct.Pin = LED1_Pin|LED0_Pin;  //引脚号
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   //推挽输出
  GPIO_InitStruct.Pull = GPIO_PULLUP;   //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;   //中速（12.5-50MHz）
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/** 
    @brief LED（绿色）控制函数
    @param flag=0：熄灭（高电平)；flag=1：点亮（低电平）
    @return NULL
*/
void led_green(uint8_t flag)
{
    switch (flag)
    {
    case 0:
        HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
        break;
    
    case 1:
        HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
        break;
    }
}

/** 
    @brief LED（红色）控制函数
    @param flag=0：熄灭（高电平)；flag=1：点亮（低电平）
    @return NULL
*/
void led_red(uint8_t flag)
{
    switch (flag)
    {
    case 0:
        HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
        break;
    
    case 1:
        HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
        break;
    }
}

/** 
    @brief LED（红色）翻转函数
    @param NULL
    @return NULL
*/
void led_red_toggle()
{
    HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
}

/**
    @brief LED（绿色）翻转函数
    @param NULL
    @return NULL
*/
void led_green_toggle()
{
    HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
}

/** 
    @brief LED自检函数
    @param NULL
    @return NULL
*/
void led_test()
{
    int i;
    for (i=0;i<999;i++)
    {
        led_green_toggle();
        led_red_toggle();
        HAL_Delay(100);
    }
}