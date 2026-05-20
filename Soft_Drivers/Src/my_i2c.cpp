#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_i2c.hpp"

#define scl_write(x)    HAL_GPIO_WritePin(this->gpiox, this->scl, (GPIO_PinState)x)
#define sda_write(x)    HAL_GPIO_WritePin(this->gpiox, this->sda, (GPIO_PinState)x)

my_i2c::my_i2c(std::string _self_name, GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda) : self_name(_self_name), gpiox(_gpiox), scl(_scl), sda(_sda)
{
    scl_write(1);
    sda_write(1);
}

int my_i2c::enable_delay(void)
{
    is_delay = 1;
    return 0;
}

string my_i2c::get_name(void)
{
    return self_name;
}

//==============================================================================
//  写: START + 设备地址 + ACK + 寄存器地址 + ACK + 数据... + STOP
//==============================================================================
int my_i2c::send_data(unsigned char dev_addr_w, unsigned char reg_addr,
                      unsigned char *send_data, int len)
{
    start();
    
    if(send_byte(dev_addr_w))  { stop(); return -1; }
    if(send_byte(reg_addr))    { stop(); return -1; }
    
    for(int i = 0; i < len; i++)
    {
        if(send_byte(send_data[i]))  { stop(); return -1; }
    }
    
    stop();
    return 0;
}

//==============================================================================
//  读: START + 设备地址(w) + ACK + 寄存器地址 + ACK +
//       RESTART + 设备地址(r) + ACK + 数据... + NACK + STOP
//==============================================================================
int my_i2c::receive_data(unsigned char dev_addr_w, unsigned char reg_addr,
                         unsigned char *receive_data, int len)
{
    start();
    
    if(send_byte(dev_addr_w))         { stop(); return -1; }
    if(send_byte(reg_addr))           { stop(); return -1; }
    
    start();
    
    if(send_byte(dev_addr_w | 0x01))  { stop(); return -1; }
    
    for(int i = 0; i < len; i++)
    {
        receive_data[i] = receive_byte();
        if(i == len - 1)
        {
            send_ack(1);
        }
        else
        {
            send_ack(0);
        }
    }
    
    stop();
    return 0;
}

//==============================================================================
//  底层时序
//==============================================================================

int my_i2c::start(void)
{
    sda_write(1);
    scl_write(1);
    sda_write(0);
    scl_write(0);
    return 0;
}

int my_i2c::stop(void)
{
    sda_write(0);
    scl_write(0);
    scl_write(1);
    sda_write(1);
    return 0;
}

int my_i2c::send_byte(unsigned char byte)
{
    scl_write(0);
    sda_write(0);
    
    for(int i = 0; i < 8; i++)
    {
        scl_write(0);
        if(byte & (0x80 >> i))
        {
            sda_write(1);
        }
        else
        {
            sda_write(0);
        }
        scl_write(1);
        scl_write(0);
    }
    sda_write(1);
    
    return receive_ack();  // receive_ack 内部释放 SDA + 产生 SCL 脉冲 + 读 ACK
}

unsigned char my_i2c::receive_byte(void)
{
    unsigned char data = 0x00;
    
    sda_write(1);
    for(int i = 0; i < 8; i++)
    {
        scl_write(1);
        if(HAL_GPIO_ReadPin(gpiox, sda))
        {
            data |= (0x80 >> i);
        }
        scl_write(0);
    }
    
    return data;
}

int my_i2c::send_ack(unsigned char ack)
{
    scl_write(0);
    if(ack)
    {
        sda_write(1);
    }
    else
    {
        sda_write(0);
    }
    scl_write(1);
    scl_write(0);
    return 0;
}

unsigned char my_i2c::receive_ack(void)
{
    unsigned char ack;
    
    scl_write(0);
    sda_write(1);
    if(is_delay)
    {
        HAL_Delay(1);
    }
    scl_write(1);
    if(is_delay)
    {
        HAL_Delay(1);
    }
    ack = HAL_GPIO_ReadPin(gpiox, sda);
    scl_write(0);
    
    return ack;  // 0=ACK, 1=NACK
}
