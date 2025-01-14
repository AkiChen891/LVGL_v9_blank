/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      ALIENTEK, recode by Chen Cheng
 * @version     V1.1
 * @date        2024-10-25
 * @brief       系统延时函数
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#include "sys.h"
#include "delay.h"


static uint32_t g_fac_us = 0;       /* us延时倍乘数 */

/* 如果该宏被定义，说明即将支持OS */
#if SYS_SUPPORT_OS

/* 公共头文件 */
#include "os.h"

/* ����g_fac_ms����, ��ʾms��ʱ�ı�����, ����ÿ�����ĵ�ms��, (����ʹ��os��ʱ��,��Ҫ�õ�) */
static uint16_t g_fac_ms = 0;

/*
 *  ��delay_us/delay_ms��Ҫ֧��OS��ʱ����Ҫ������OS��صĺ궨��ͺ�����֧��
 *  ������3���궨��:
 *      delay_osrunning    :���ڱ�ʾOS��ǰ�Ƿ���������,�Ծ����Ƿ����ʹ����غ���
 *      delay_ostickspersec:���ڱ�ʾOS�趨��ʱ�ӽ���,delay_init�����������������ʼ��systick
 *      delay_osintnesting :���ڱ�ʾOS�ж�Ƕ�׼���,��Ϊ�ж����治���Ե���,delay_msʹ�øò����������������
 *  Ȼ����3������:
 *      delay_osschedlock  :��������OS�������,��ֹ����
 *      delay_osschedunlock:���ڽ���OS�������,���¿�������
 *      delay_ostimedly    :����OS��ʱ,���������������.
 *
 *  �����̽���UCOSII��֧��,����OS,�����вο���ֲ
 */
 
/* ֧��UCOSII */
#define delay_osrunning     OSRunning           /* OS�Ƿ����б��,0,������;1,������ */
#define delay_ostickspersec OS_TICKS_PER_SEC    /* OSʱ�ӽ���,��ÿ����ȴ��� */
#define delay_osintnesting  OSIntNesting        /* �ж�Ƕ�׼���,���ж�Ƕ�״��� */

/**
 * @brief     us����ʱʱ,�ر��������(��ֹ���us���ӳ�)
 * @param     ��  
 * @retval    ��
 */  
void delay_osschedlock(void)
{
    OSSchedLock();                      /* UCOSII�ķ�ʽ,��ֹ���ȣ���ֹ���us��ʱ */
}

/**
 * @brief     us����ʱʱ,�ָ��������
 * @param     ��
 * @retval    ��
 */  
void delay_osschedunlock(void)
{
    OSSchedUnlock();                    /* UCOSII�ķ�ʽ,�ָ����� */
}

/**
 * @brief     us����ʱʱ,�ָ��������
 * @param     ticks : ��ʱ�Ľ�����
 * @retval    ��
 */  
void delay_ostimedly(uint32_t ticks)
{
    OSTimeDly(ticks);                               /* UCOSII��ʱ */ 
}

/**
 * @brief     systick�жϷ�����,ʹ��OSʱ�õ�
 * @param     ticks : ��ʱ�Ľ�����  
 * @retval    ��
 */  
void SysTick_Handler(void)
{
    if (delay_osrunning == OS_TRUE) /* OS��ʼ����,��ִ�������ĵ��ȴ��� */
    {
        OS_CPU_SysTickHandler();    /* ���� uC/OS-II �� SysTick �жϷ����� */
    }
    HAL_IncTick();
}
#endif

/**
 * @brief     延时函数初始化
 * @param     sysclk: 系统时钟（HCLK）频率，单位MHz
 * @retval    无
 */  
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS                                      /* 如果需要支持OS */
    uint32_t reload;
#endif
    g_fac_us = sysclk;                                  
#if SYS_SUPPORT_OS                                      
    reload = sysclk;                                    
    reload *= 1000000 / delay_ostickspersec;            
                                                           
                                                         
    g_fac_ms = 1000 / delay_ostickspersec;              
    SysTick->CTRL |= 1 << 1;                            
    SysTick->LOAD = reload;                            
    SysTick->CTRL |= 1 << 0;                            
#endif 
}


/**
 * @brief     延时函数（微妙）
 * @note      时钟摘取法
 * @param     nus: 延时值（微妙）
 * @note      取值范围：0 ~ (2^32 / fac_us)，fac_us一般等于sysclk频率
 * @retval    无
 */ 
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        
    ticks = nus * g_fac_us;                 /* 需要的tick数 */

#if SYS_SUPPORT_OS                          
    delay_osschedlock();                    
#endif

    told = SysTick->VAL;                   
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;        
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) 
            {
                break;                      
            }
        }
    }

#if SYS_SUPPORT_OS                          
    delay_osschedunlock();                  
#endif
} 

/**
 * @brief     延时函数（毫秒）
 * @param     nms: 延时值（毫秒）
 * @note      取值范围：0 ~ (2^32 / fac_us / 1000) fac_us一般等于sysclk频率
 * @retval    无
 */
void delay_ms(uint16_t nms)
{
#if SYS_SUPPORT_OS  
    if (delay_osrunning && delay_osintnesting == 0)     
    {
        if (nms >= g_fac_ms)                            
        {
            delay_ostimedly(nms / g_fac_ms);            
        }

        nms %= g_fac_ms;                                
    }
#endif

    delay_us((uint32_t)(nms * 1000));                   
}


/**
 * @brief       HAL库默认延时
 * @note        HAL库延时函数使用systick，如果未启用systick中断会导致调用该延时后无法退出
 * @param       Delay : 待延时时间（毫秒）
 * @retval      无
 */
void HAL_Delay(uint32_t Delay)
{
    delay_ms(Delay);
}










