/**
 * @file    servo_SG90.cpp
 * @brief   SG90 舵机驱动 —— 基于 PWM 的占空比控制（继承 my_pwm）
 * @date    2026-06-06
 *
 * @note    SG90 标准参数: 周期 20ms, 占空比范围 0~12.5% (对应 0°~180°)
 *          超出范围返回 -1。
 */

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "servo_SG90.hpp"

// ======================== 构造 ========================

/**
 * @brief 构造函数 —— 初始化 PWM 输出
 * @param _htim     TIM 句柄
 * @param _channel  PWM 通道
 * @param _duty     初始占空比 (%)
 */
servo_sg90::servo_sg90(TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty)
: my_pwm("servo_sg90_pwm", _htim, _channel, _duty)
{
    // 基类 my_pwm 自动启动定时器和 PWM 输出
}

// ======================== 控制接口 ========================

/**
 * @brief 设置舵机角度（通过占空比）
 * @param duty  占空比 (%), 有效范围 0 ~ 12.5
 * @return 0=成功, -1=超出范围
 */
int servo_sg90::set_duty(float duty)
{
    // 范围检查: SG90 标准 0~12.5%
    if(duty < 0.0f || duty > 12.5f)
    {
        return -1;
    }

    auto res = my_pwm::set_duty(duty);
    if(res != 0)
    {
        return res;
    }

    return 0;
}