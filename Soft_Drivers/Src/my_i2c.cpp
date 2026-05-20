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
int my_i2c::send_data(unsigned char dev_addr_w, unsigned char reg_addr, unsigned char *send_data, int len)
{
    start();
    send_byte(dev_addr_w);
    if(receive_ack())
    {
        stop();
        
        return -1;
    }
    for(auto i = 0; i < len; i++)
    {
        send_byte(*send_data);
        send_data++;
    }
    stop();

    return 0;
}
int my_i2c::receive_data(unsigned char dev_addr_w, unsigned char reg_addr, unsigned char *receive_data, int len)
{
    start();
    send_byte(dev_addr_w);
    if(receive_ack())
    {
        stop();

        return -1;
    }
    send_byte(reg_addr);
    if(receive_ack())
    {
        stop();

        return -1;
    }
    start();
    send_byte((dev_addr_w << 1 | 0x01));
    if(receive_ack())
    {
        stop();

        return -1;
    }
    for(auto i = 0; i < len; i++)
    {
        *receive_data = receive_byte();
        if(i + 1 < len)
        {
            send_ack(0);
        }
        else
        {
            send_ack(1);
        }
        receive_data++;
    }
    stop();

    return 0;
}
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
    int ack;
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

    return ack;
}
int my_i2c::send_byte(unsigned char byte)
{
    scl_write(0);
    sda_write(0);
    for(auto i = 0; i < 8; i++)
    {
        scl_write(0);
        if((byte << i) & 0x80)
        {
            sda_write(1);
        }
        scl_write(1);
        scl_write(0);
        sda_write(0);
    }
    sda_write(1);
    scl_write(1);
    int ack = receive_ack(); 
    scl_write(0);
    
    if(ack)
    {
        return -1;
    }
    
    return 0;
}
unsigned char my_i2c::receive_byte(void)
{
    unsigned char receive_data = 0x00;
    for(auto i = 0; i < 8; i++)
    {
        scl_write(1);
        if(HAL_GPIO_ReadPin(gpiox, sda))
        {
            receive_data |= (0x80 >> i);
        }
        scl_write(0);
    }

    return receive_data;
}
