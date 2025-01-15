/**
 ****************************************************************************************************
 * @file        24cxx.c
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-04-20
 * @brief       24CXX 驱动代码
 ****************************************************************************************************
 * @attention
 *              IDE不支持半主机模式时应屏蔽代码中所有printf()语句
 *
 ****************************************************************************************************
 */

#include "myiic.h"
#include "24cxx.h"
#include "delay.h"


/**
 * @brief       初始化IIC接口
 * @param       NULL
 * @retval      NULL
 */
void at24cxx_init(void)
{
    iic_init();
}

/**
 * @brief       �在AT24CXX指定地址读出一个数据
 * @param       addr: 开始读数的地址
 * @retval      读到的数据
 */
uint8_t at24cxx_read_one_byte(uint16_t addr)
{
    uint8_t temp = 0;
    iic_start();    /* 发送起始信号 */

    /* 根据不同的24CXX型号, 发送高位地址
     * 1, 24C16以上的型号, 分2个字节发送地址
     * 2, 24C16及以下型号, 分1个低字节地址 + 占用器件地址的bit1~bit3 用于表示高位地址，最多11位地址
     *    对于24C01/02, 器件格式地址(8bit): 1  0  1  0  A2  A1  A0  R/W
     *    对于24C04,    器件格式地址(8bit): 1  0  1  0  A2  A1  a8  R/W
     *    对于24C08,    器件格式地址(8bit): 1  0  1  0  A2  a9  a8  R/W
     *    对于24C16,    器件格式地址(8bit): 1  0  1  0  a10 a9  a8  R/W
     *    R/W      : 读/写控制位 0,表示写 1,表示读;
     *    A0/A1/A2 : 硬件引脚
     *    a8/a9/a10: 对应存储整列的高位地址，11bit地址最多可表示2048个为止，可寻址24C16及以下型号
     */    
    if (EE_TYPE > AT24C16)                          /* 24C16以上型号分2个字节发送地址ַ */
    {
        iic_send_byte(0xA0);                        /* 发送写命令，IIC规定最低位是0，表示写入 */
        iic_wait_ack();                             /* 每次发送完一个字节都需等待ACK */
        iic_send_byte(addr >> 8);                   /* 发送高字节地址 */
    }
    else 
    {
        iic_send_byte(0xA0 + ((addr >> 8) << 1));   /* 发送器件 0xA0 + 高位a8/a9/a10地址，写数据 */
    }
    
    iic_wait_ack();                                 /* 每次发送完一个字节，都需等待ACK */
    iic_send_byte(addr % 256);                      /* 发送低位地址 */
    iic_wait_ack();                                 /* 等待ACK */
    
    iic_start();                                    /* 重新发送起始信号 */ 
    iic_send_byte(0xA1);                            /* 进入接收模式 */
    iic_wait_ack();                                 /* 等待ACK */
    temp = iic_read_byte(0);                        /* 接收一个字节数据 */
    iic_stop();                                     /* IIC停止 */

    return temp;
}

/**
 * @brief       AT24CXX指定地址写入数据
 * @param       addr: 写入数据目的地址
 * @param       data: 待写入数据
 * @retval      NULL
 */
void at24cxx_write_one_byte(uint16_t addr, uint8_t data)
{
    iic_start();                                    

    if (EE_TYPE > AT24C16)                          
    {
        iic_send_byte(0xA0);                       
        iic_wait_ack();                            
        iic_send_byte(addr >> 8);                
    }
    else
    {
        iic_send_byte(0xA0 + ((addr >> 8) << 1));   
    }
    
    iic_wait_ack();                                 
    iic_send_byte(addr % 256);                      
    iic_wait_ack();                                
    
    /* IIC已经位于接收模式，无需重复发送起始信号 */
    iic_send_byte(data);                            
    iic_wait_ack();                                 
    iic_stop();                                     
    delay_ms(10);                                   /* 等待EEPROM写入完成 */
}
 
/**
 * @brief       检查AT24CXX是否正常
 * @param       NULL
 * @retval      检测结果
 *              0: 检测成功
 *              1: 检测失败
 */
uint8_t at24cxx_check(void)
{
    uint8_t temp;
    uint16_t addr = EE_TYPE;

    temp = at24cxx_read_one_byte(addr);     /* 避免每次开机都写AT24CXX */
    if (temp == 0x55)                       /* 读取数据正常 */
    {
        return 0;
    }
    else                                    /* 排除第一次初始化的情况 */
    {
        at24cxx_write_one_byte(addr, 0x55); /* 写入 */
        temp = at24cxx_read_one_byte(255);  /* 读取 */

        if (temp == 0x55)
        {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief       AT24CXX从指定地址开始读出指定个数数据
 * @param       addr    : 开始读出的地址，24c02为0~255
 * @param       pbuf    : 缓冲区首地址
 * @param       datalen : 待读取数据长度
 * @retval      NULL
 */
void at24cxx_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    while (datalen--)
    {
        *pbuf++ = at24cxx_read_one_byte(addr++);
    }
}

/**
 * @brief       AT24CXX指定地址开始写入指定个数的数据
 * @param       addr    : 开始写入的地址
 * @param       pbuf    : 缓冲区首地址
 * @param       datalen : 待写入数据长度
 * @retval      NULL
 */
void at24cxx_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    while (datalen--)
    {
        at24cxx_write_one_byte(addr, *pbuf);
        addr++;
        pbuf++;
    }
}






