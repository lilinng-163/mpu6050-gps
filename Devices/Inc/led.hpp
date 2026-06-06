/**
 * @file    led.hpp
 * @brief   LED 驱动声明 —— GPIO 控制板载 LED
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"

/**
 * @brief LED 驱动类（低电平点亮）
 */
class led
{
public:
    led(GPIO_TypeDef *_gpiox, uint16_t _pin);
    int on(void);              /**< 点亮 LED */
    int off(void);             /**< 熄灭 LED */
    bool get_status(void);     /**< 查询亮灭状态 */
private:
    bool is_on;                /**< 当前状态 */
    GPIO_TypeDef *gpiox;       /**< GPIO 端口 */
    uint16_t pin;              /**< GPIO 引脚 */
};