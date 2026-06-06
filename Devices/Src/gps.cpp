/**
 * @file    gps.cpp
 * @brief   GPS 模块驱动 —— 环形缓冲区管理，原始 NMEA 数据 dump
 * @date    2026-06-06
 *
 * @note    GPS 原始字节由 USART3 中断回调推入 rx_buf 环形缓冲区，
 *          本模块提供 dump_raw() 消费全部字节、get_data() 解析（待实现）。
 */

#include <string>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "gps.hpp"

using namespace etl;
using std::string;

// ======================== 构造 / 析构 ========================

gps_module::gps_module(void)
{
    rx_buf.clear();
}

// ======================== 数据操作 ========================

/**
 * @brief 从环形缓冲区中取出所有原始字节
 * @param buffer  输出字符串
 * @return 0=成功
 *
 * 在临界区中一次性消费缓冲区中的所有字节，拼接为字符串。
 */
int gps_module::dump_raw(string &buffer)
{
    taskENTER_CRITICAL();
    if(!buffer.empty())
    {
        buffer.clear();
    }

    while (rx_buf.size() > 0)
    {
        buffer += rx_buf.front();
        rx_buf.pop();
    }

    taskEXIT_CRITICAL();
    return 0;
}

/**
 * @brief 逐字节喂入 GPS 数据（预留，当前未使用）
 */
bool gps_module::feed(unsigned char byte)
{
    return true;
}

/**
 * @brief 解析 GPS 数据（待实现）
 */
int gps_module::get_data(gps_data_t &gps_data)
{
    return 0;
}
