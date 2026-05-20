#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"

using std::string;

class my_i2c
{
public:
    my_i2c(string _self_name, GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int enable_delay(void);
    string get_name(void);
    int send_data(unsigned char dev_addr_w, unsigned char reg_addr, unsigned char *send_data, int len);
    int receive_data(unsigned char dev_addr_w, unsigned char reg_addr, unsigned char *receive_data, int len);
private:
    string self_name;
    GPIO_TypeDef *gpiox;
    uint16_t scl;
    uint16_t sda;
    int is_delay = 0;
    int start(void);
    int stop(void);
    int send_ack(unsigned char ack);
    unsigned char receive_ack(void);
    int send_byte(unsigned char byte);
    unsigned char receive_byte(void);
};