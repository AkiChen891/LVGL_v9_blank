/**
 ****************************************************************************************************
 * @file        sys.c
 * @author      ALIENTEK，recode by Chen Cheng
 * @version     V1.0
 * @date        2022-4-20
 * @brief       系统初始化代码
 ****************************************************************************************************
 * @attention
 *              不要改动此文件中的任何内容！
 * 
 *              调用本文件中的函数前，确保知道你在干什么
 ****************************************************************************************************
 */

#include "sys.h"


/**
 * @brief       设置中断向量表偏移地址
 * @param       baseaddr: 基准地址
 * @param       offset: 偏移量
 * @retval      无
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* 设置NVIC的向量表偏移寄存器，VTOR低9位保留 */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

/**
 * @brief       执行WFI指令，执行后MCU进入低功耗状态，等待中断唤醒
 * @param       无
 * @retval      无
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       关闭所有中断(不包括fault和NMI中断)
 * @param       无
 * @retval      无
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       打开所有中断
 * @param       无
 * @retval      无
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       设置栈顶地址
 * @note        使用MDK编译时可能出现误报
 * @param       addr: 栈顶地址
 * @retval      无
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);    /* 设置栈顶地址 */
}

/**
 * @brief       MCU进入待机模式
 * @param       无
 * @retval      无
 */
void sys_standby(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();    /* 使能电源时钟 */
    SET_BIT(PWR->CR, PWR_CR_PDDS); /* 进入待机模式 */
}

/**
 * @brief       系统软复位
 * @param       无
 * @retval      无
 */
void sys_soft_reset(void)
{
    NVIC_SystemReset();
}


/* 注意：不建议使用该函数进行时钟设置，推荐在CubeMX中进行可视化配置 */
/**
 * @brief       时钟设置函数
 * @param       plln: PLL1预分频系数（PLL倍频），取值范围：64-432
 * @param       pllm: PLL1预分频系数（进PLL之前的分频）, 取值范围: 2~63.
 * @param       pllp: PLL1的p分频系数（PLL之后的分频），分频后作为sysclk，取值范围：2，4，6，8
 * @param       pllq: PLL1的q分频系数（PLL之后的分频），取值范围：2~15
 * @note
 *
 *              不建议使用该函数进行时钟配置！
 * 
 *              当外部高速晶振为25MHz时，推荐sysclk频率180MHz，PLL source选择HSE，/M为25，*N为360，sysclk来源选择PLLCLK；
 *              APB1预分频为/4，APB2预分频为/2
 *
 * @retval      错误代码，0：成功；1：错误；
 */
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_ClkInitTypeDef rcc_clk_init = {0};
    RCC_OscInitTypeDef rcc_osc_init = {0};
    
    __HAL_RCC_PWR_CLK_ENABLE();                                     /* 使能PWR时钟 */
    
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);  /* 调压器输出电压级别为模式1 */

    /* 使能HSE，选择HSE作为PLL时钟源，配置PLL1，开启USB时钟 */
    rcc_osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;           /* 时钟源为HSE */
    rcc_osc_init.HSEState = RCC_HSE_ON;                             /* 打开HSE */
    rcc_osc_init.PLL.PLLState = RCC_PLL_ON;                         /* 打开PLL */
    rcc_osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;                 /* PLL时钟源为HSE */
    rcc_osc_init.PLL.PLLN = plln;
    rcc_osc_init.PLL.PLLM = pllm;
    rcc_osc_init.PLL.PLLP = pllp;
    rcc_osc_init.PLL.PLLQ = pllq;
    ret = HAL_RCC_OscConfig(&rcc_osc_init);                         /* 初始化RCC */
    if (ret != HAL_OK)
    {
        return 1;                                                   
    }

    ret = HAL_PWREx_EnableOverDrive();                              /* 打开Over-Driver */
    if (ret != HAL_OK)
    {
        return 1;
    }

    /* sysclk时钟源为PLL */
    rcc_clk_init.ClockType = ( RCC_CLOCKTYPE_SYSCLK \
                                    | RCC_CLOCKTYPE_HCLK \
                                    | RCC_CLOCKTYPE_PCLK1 \
                                    | RCC_CLOCKTYPE_PCLK2);

    rcc_clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;            /* 配置sysclk时钟源为PLLCLK */
    rcc_clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;                   /* AHB分频系数 */
    rcc_clk_init.APB1CLKDivider = RCC_HCLK_DIV4;                    /* APB1分频系数 */
    rcc_clk_init.APB2CLKDivider = RCC_HCLK_DIV2;                    /* APB2分频系数 */
    ret = HAL_RCC_ClockConfig(&rcc_clk_init, FLASH_LATENCY_5);      /* FLASH延时为6个CPU周期 */
    if (ret != HAL_OK)
    {
        return 1;                                                   
    }

    return 0;
}


#ifdef  USE_FULL_ASSERT

/**
 * @brief       编译报错时该函数用于回报错误文件和所在行
 * @param       file：指向源文件
 * @param       line：指向所在文件中的行数
 * @retval      无
 */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    while (1)
    {
    }
}
#endif




