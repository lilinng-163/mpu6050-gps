/**
 * @file    my_pwm.cpp
 * @brief   通用 PWM 驱动 —— 基于 STM32 HAL 定时器的占空比/周期控制
 * @date    2026-06-06
 */

#include <string>
#include <cstdio>
#include <cstdint>
#include <stm32f1xx_hal.h>
#include "tim.h"
#include "my_pwm.hpp"

// ======================== 构造 ========================

/**
 * @brief 构造函数 —— 启动定时器和 PWM 输出
 * @param _self_name  实例名称
 * @param _htim       TIM 句柄
 * @param _channel    PWM 通道
 * @param _duty       初始占空比 (%)
 */
my_pwm::my_pwm(std::string _self_name, TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty)
: self_name(_self_name), htim(_htim), channel(_channel), duty(_duty), arr(__HAL_TIM_GET_AUTORELOAD(htim))
{
    // 启动定时器基频
    HAL_TIM_Base_Start(htim);
    // 启动 PWM 输出
    HAL_TIM_PWM_Start(htim, channel);
}

// ======================== 控制接口 ========================

/**
 * @brief 设置 PWM 占空比
 * @param duty  占空比 (%), 范围 0~100
 * @return 0=成功, -1=超出范围
 */
int my_pwm::set_duty(float duty)
{
    if(duty < 0.0f || duty > 100.0f )
    {
        return -1;
    }

    this->duty = duty;

    // 占空比 → 比较值: pulse = duty% × ARR
    uint32_t pulse = (uint32_t)(duty / 100.0f * this->arr);
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);

    return 0;
}

/**
 * @brief 设置 PWM 周期（自动重装载值）
 * @param arr  ARR 值
 * @return 0
 */
int my_pwm::set_arr(uint32_t arr)
{
    this->arr = arr;
    __HAL_TIM_SET_AUTORELOAD(htim, arr);
    return 0;
}