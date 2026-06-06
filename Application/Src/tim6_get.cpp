/**
 * @file    tim6_get.cpp
 * @brief   FreeRTOS 运行时统计时钟 —— 使用 TIM6 (16-bit) 作为高分辨率计时基准
 * @date    2026-06-06
 *
 * @note    FreeRTOS 要求提供两个函数用于 run-time stats:
 *          - vConfigureTimerForRunTimeStats(): 初始化时钟（TIM6 由 CubeMX 初始化，此处为空）
 *          - ulGetRunTimeCounterValue(): 返回 32-bit 时间戳
 *          TIM6 为 16-bit 自增计数器，本实现通过软件扩展为 32-bit。
 */

#include <cstdint>
#include "tim6_get.hpp"

/**
 * @brief 配置运行时统计时钟（TIM6 已由 CubeMX 初始化，此处无操作）
 */
void vConfigureTimerForRunTimeStats(void)
{
    // TIM6 由 CubeMX 生成的代码初始化
}

/**
 * @brief 获取 32-bit 运行时间计数值
 * @return 扩展后的 32-bit 时间戳
 *
 * TIM6 是 16-bit 计数器，溢出时 now < last，通过 high 变量记录溢出次数，
 * 组合为 32-bit 值: high << 16 | now
 */
uint32_t ulGetRunTimeCounterValue(void)
{
    static uint16_t last = 0;    /**< 上一次读取的 CNT 值 */
    static uint32_t high = 0;    /**< 高 16 位（溢出累计） */
    uint16_t now = TIM6->CNT;    /**< 当前计数器值 */

    // 检测 16-bit 溢出
    if(now < last)
    {
        high += 0x10000;         // 溢出一次 = +65536
    }
    last = now;

    return high + now;
}