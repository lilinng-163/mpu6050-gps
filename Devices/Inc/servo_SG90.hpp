/**
 * @file    servo_SG90.hpp
 * @brief   SG90 舵机驱动声明 —— 基于 PWM 的角度控制（继承 my_pwm）
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_pwm.hpp"

using std::string;

/**
 * @brief SG90 舵机驱动类
 *
 * PWM 参数: 周期 20ms, 占空比 0~12.5% 对应 0°~180°
 */
class servo_sg90 : public my_pwm
{
public:
    servo_sg90(TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty);
    int set_duty(float duty);          /**< 设置占空比 (%), 范围 0~12.5 */
private:
    uint16_t period = 20;              /**< PWM 周期 (ms) */
    float max_duty = 12.5;             /**< 最大占空比 (%) */
};