/**
 * @file    uart_cb.cpp
 * @brief   UART 中断回调 —— USART3 接收 GPS 字节写入环形缓冲区，USART2 预留 ESP32 通信
 * @date    2026-06-06
 *
 * @note    USART3 → GPS 模块 (单字节中断接收 → gps_module::rx_buf)
 *          USART2 → ESP32   (预留)
 */

#include <string>
#include <cstdint>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "gps.hpp"

using namespace etl;
using std::string;

/** GPS 单字节接收缓冲区 */
uint8_t gps_rx_byte = 0x00;
/** 全局 GPS 模块实例（环形缓冲在其中） */
gps_module gm;

/**
 * @brief HAL UART 接收完成回调
 *
 * USART3: GPS 数据 → 推入 gm.rx_buf 环形缓冲区 → 重新启动中断接收
 * USART2: ESP32 通信（预留）
 */
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        // GPS 字节入环形缓冲区，并立即重启 DMA/中断接收
        gm.rx_buf.push((char)gps_rx_byte);
        HAL_UART_Receive_IT(&huart3, &gps_rx_byte, 1);
    }
    if(huart->Instance == USART2)
    {
        // ESP32 通信预留
    }
}