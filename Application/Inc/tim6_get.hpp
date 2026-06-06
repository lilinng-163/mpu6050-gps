/**
 * @file    tim6_get.hpp
 * @brief   FreeRTOS 运行时统计时钟接口 —— 基于 TIM6 的 32-bit 时间戳
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>
#include <string>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 配置运行时统计时钟（TIM6 由 CubeMX 初始化）
 */
void vConfigureTimerForRunTimeStats(void);

/**
 * @brief 获取 32-bit 运行时间计数值
 * @return 扩展后的 32-bit 时间戳
 */
uint32_t ulGetRunTimeCounterValue(void);

#ifdef __cplusplus
}
#endif