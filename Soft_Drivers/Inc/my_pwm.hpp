/**
 * @file    my_pwm.hpp
 * @brief   通用 PWM 驱动声明 —— 基于 STM32 HAL 定时器
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"

using std::string;

/**
 * @brief 通用 PWM 驱动类
 */
class my_pwm
{
public:
    my_pwm(std::string _self_name, TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty);
    int set_duty(float duty);          /**< 设置占空比 (%), 0~100 */
    int set_arr(uint32_t arr);         /**< 设置自动重装载值 (周期) */
private:
    string self_name;                  /**< 实例名称 */
    TIM_HandleTypeDef *htim;           /**< 定时器句柄 */
    uint32_t channel;                  /**< PWM 通道 */
    float duty;                        /**< 当前占空比 (%) */
    uint32_t arr;                      /**< 自动重装载值 */
};