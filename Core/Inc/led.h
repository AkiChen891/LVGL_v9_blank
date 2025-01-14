/**
  ******************************************************************************
  * @file           : led.h
  * @brief          : LED函数头文件
  ******************************************************************************
  * @attention
  *
  *     此文件应当在main.c中被include
  ******************************************************************************
  */

#include "main.h"

void led_init();
void led_green(int flag);
void led_red(int flag);
void led_red_toggle();
void led_green_toggle();
