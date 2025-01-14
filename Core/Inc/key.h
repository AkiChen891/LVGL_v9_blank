/**
  ******************************************************************************
  * @file           : key.h
  * @brief          : 按键函数头文件
  ******************************************************************************
  * @attention
  *
  *     此文件应当在main.c中被include
  ******************************************************************************
  */
 #include <stdio.h>
 #include "main.h"

/* KEY2对应PC3 */
#define KEY2_Pin GPIO_PIN_3
#define KEY2_GPIO_Port GPIOC
/* WKUP对应PA0 */
#define WKUP_Pin GPIO_PIN_0
#define WKUP_GPIO_Port GPIOA
/* KEY1对应PH2 */
#define KEY1_Pin GPIO_PIN_2
#define KEY1_GPIO_Port GPIOH
/* KEY0对应PH3 */
#define KEY0_Pin GPIO_PIN_3
#define KEY0_GPIO_Port GPIOH

#define KEY0 HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin) //读取KEY0引脚电平
#define KEY1 HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) //读取KEY1引脚电平
#define KEY2 HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin) //读取KEY2引脚电平
#define WKUP HAL_GPIO_ReadPin(WKUP_GPIO_Port,WKUP_Pin) //读取WKUP引脚电平

#define KEY0_pressed 1  //KEY0按下
#define KEY1_pressed 2  //KEY1按下
#define KEY2_pressed 3  //KEY2按下
#define WKUP_pressed 4  //WKUP按下

void key_init();
uint8_t key_scan(uint8_t mode);