/**
 ****************************************************************************************************
 * @file        ctiic.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-04-20
 * @brief       ���ݴ����� ��������
 * @license     Copyright (c) 2020-2032, �������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ F429������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220420
 * ��һ�η���
 *
 ****************************************************************************************************
 */
 
#ifndef __MYCT_IIC_H
#define __MYCT_IIC_H

#include "sys.h"   


/******************************************************************************************/

/* CT_IIC ���� ���� */
#define CT_IIC_SCL_GPIO_PORT              GPIOH
#define CT_IIC_SCL_GPIO_PIN               GPIO_PIN_6
#define CT_IIC_SCL_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOH_CLK_ENABLE(); }while(0)    /* PH��ʱ��ʹ�� */

#define CT_IIC_SDA_GPIO_PORT              GPIOI
#define CT_IIC_SDA_GPIO_PIN               GPIO_PIN_3
#define CT_IIC_SDA_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOI_CLK_ENABLE(); }while(0)    /* PI��ʱ��ʹ�� */

/******************************************************************************************/

/* IO���� */
#define CT_IIC_SCL(x)       do{ x ? \
                                   HAL_GPIO_WritePin(CT_IIC_SCL_GPIO_PORT, CT_IIC_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                                   HAL_GPIO_WritePin(CT_IIC_SCL_GPIO_PORT, CT_IIC_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                               }while(0)                                                 /* SCL���� */

#define CT_IIC_SDA(x)       do{ x ? \
                                   HAL_GPIO_WritePin(CT_IIC_SDA_GPIO_PORT, CT_IIC_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                                   HAL_GPIO_WritePin(CT_IIC_SDA_GPIO_PORT, CT_IIC_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                               }while(0)                                                /* SDA���� */

#define CT_READ_SDA         HAL_GPIO_ReadPin(CT_IIC_SDA_GPIO_PORT, CT_IIC_SDA_GPIO_PIN) /* ����SDA */

/******************************************************************************************/

/* IIC���в������� */
void ct_iic_init(void);                      /* ��ʼ��IIC��IO�� */
void ct_iic_start(void);                     /* ����IIC��ʼ�ź� */
void ct_iic_stop(void);                      /* ����IICֹͣ�ź� */
void ct_iic_send_byte(uint8_t data);         /* IIC����һ���ֽ� */
uint8_t ct_iic_read_byte(unsigned char ack); /* IIC��ȡһ���ֽ� */
uint8_t ct_iic_wait_ack(void);               /* IIC�ȴ�ACK�ź� */
void ct_iic_ack(void);                       /* IIC����ACK�ź� */
void ct_iic_nack(void);                      /* IIC������ACK�ź� */

#endif







