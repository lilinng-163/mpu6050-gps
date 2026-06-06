/**
 * @file    led.cpp
 * @brief   LED 驱动 —— GPIO 控制板载 LED 的亮/灭/状态查询
 * @date    2026-06-06
 *
 * @note    低电平点亮（GPIO_PIN_RESET = ON），高电平熄灭。
 *          构造函数读取当前 GPIO 电平以初始化状态。
 */

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "led.hpp"

// ======================== 构造 ========================

/**
 * @brief 构造函数 —— 读取当前引脚电平确定初始状态
 * @param _gpiox  GPIO 端口
 * @param _pin    GPIO 引脚号
 */
led::led(GPIO_TypeDef *_gpiox, uint16_t _pin)
: gpiox(_gpiox), pin(_pin)
{
    // 低电平 = 点亮, 高电平 = 熄灭
    is_on = !(bool)HAL_GPIO_ReadPin(gpiox, pin);
}

// ======================== 控制接口 ========================

/** @brief 点亮 LED（输出低电平） */
int led::on(void)
{
    HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_RESET);
    is_on = true;
    return 0;
}

/** @brief 熄灭 LED（输出高电平） */
int led::off(void)
{
    HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_SET);
    is_on = false;
    return 0;
}

/** @brief 查询当前亮灭状态 */
bool led::get_status(void)
{
    return is_on;
}
