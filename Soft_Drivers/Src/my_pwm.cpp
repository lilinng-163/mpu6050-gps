#include <string>
#include <cstdio>
#include <cstdint>
#include <stm32f1xx_hal.h>
#include "tim.h"
#include "my_pwm.hpp"

my_pwm::my_pwm(std::string _self_name, TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty)
: self_name(_self_name), htim(_htim), channel(_channel), duty(_duty), arr(__HAL_TIM_GET_AUTORELOAD(htim))
{
    //启动定时器
    HAL_TIM_Base_Start(htim);
    //启动pwm输出
    HAL_TIM_PWM_Start(htim, channel);
}
int my_pwm::set_duty(float duty)
{
    if(duty < 0.0f || duty > 100.0f )
    {
        return -1;
    }

    this->duty = duty;

    uint32_t pulse = (uint32_t)(duty / 100.0f * this->arr);

    __HAL_TIM_SET_COMPARE(htim, channel, pulse);

    return 0;
}
int my_pwm::set_arr(uint32_t arr)
{
    this->arr = arr;
    __HAL_TIM_SET_AUTORELOAD(htim, arr);
    return 0;
}