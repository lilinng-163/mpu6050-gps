#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"

using std::string;

class my_pwm
{
public:
    my_pwm(std::string _self_name, TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty);
    int set_duty(float duty);
    int set_arr(uint32_t arr);
private:
    string self_name;
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    float duty;
    uint32_t arr;
};