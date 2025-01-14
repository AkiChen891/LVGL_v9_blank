/**
  ******************************************************************************
  * @file           : key.c
  * @brief          : 按键驱动
  * @author         ：Chen Cheng
  ******************************************************************************
  * @attention
  *
  *     此文件应当与main.c一同编译
  *     注意：按键占用PC3、PA0、PH2、PH3四个IO，如果在CubeMX中重复初始化，
  *     务必确保初始化参数完全一致，否则可能报错
  ******************************************************************************
  */
 #include "main.h"
 #include "key.h"
 #include <stdio.h>
 #include "usart.h"

/** 
    @brief 按键初始化函数
    @param NULL
    @return NULL
*/
 void key_init()
 {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 启用GPIO时钟 */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*KEY2（PC3）配置 */
    GPIO_InitStruct.Pin = KEY2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY2_GPIO_Port, &GPIO_InitStruct);

    /*WKUP（PA0）配置 */
    GPIO_InitStruct.Pin = WKUP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(WKUP_GPIO_Port, &GPIO_InitStruct);

    /*KEY1（PH2）、KEY0（PH3）配置 */
    GPIO_InitStruct.Pin = KEY1_Pin|KEY0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
 }

 /** 
    @brief 按键扫描函数
    @note 同时按下多个按键时函数存在响应优先级：WKUP > KEY2 > KEY1 > KEY0
    @param mode 0 / 1
        @arg 0，不支持连续按下，按下后只有第一次调用会返回键值，否则只有当松开后再次按下才会返回键值
        @arg 1，支持连续按，按下不放时每次调用函数均返回键值
    @return 参见key.h
    @warning  第一次调用key_scan()时应当保证所有按钮均没有被按下
*/
uint8_t key_scan(uint8_t mode)
{
    /* 注意：key_up应当为static变量并保存为全局值，否则每次key_scan()时就会刷新标记 */
    static unsigned int key_up = 1; //key_up=1为按键松开，=0为按下
    unsigned int keyval = 0;
    if (mode) key_up = 1;
    if (key_up && (KEY0 == 0) || (KEY1 == 0) || (KEY2 == 0) || (WKUP == 1))
    {
        //松开标志为1，且有按键被按下
        HAL_Delay(10);  //消抖
        key_up = 0; //标记按键为被按下

    if (KEY0 == 0) keyval = KEY0_pressed;
    if (KEY1 == 0) keyval = KEY1_pressed;
    if (KEY2 == 0) keyval = KEY2_pressed;
    if (WKUP == 1) keyval = WKUP_pressed;
    }
    else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && WKUP == 0)
    {
        //没有按键被按下时标记按键为松开
        key_up = 1;
    }

    return keyval;
}