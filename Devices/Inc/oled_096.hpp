#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_i2c.hpp"

using std::string;

class oled : public my_i2c
{
public:
    oled(GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int set_pixel(uint16_t x, uint16_t y);
    int clear_pixel(uint16_t x, uint16_t y);
    int clear(void);
    int refresh(void);
    int show_string(std::string str, uint16_t x, uint16_t y);
    int show_num(int num, uint16_t x, uint16_t y);
    ~oled(void);
private:
    unsigned char buffer[1024];
    GPIO_TypeDef *gpiox;
    uint16_t scl;
    uint16_t sda;
    int write_cmd(unsigned char cmd);
    int write_data(unsigned char data);
    int setcursor(unsigned char x, unsigned char y);
};