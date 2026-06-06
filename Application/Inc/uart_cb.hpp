/**
 * @file    uart_cb.hpp
 * @brief   UART 回调声明 —— GPS 接收字节和 gps_module 全局实例
 * @date    2026-06-06
 */

#pragma once

#include "gps.hpp"

/** GPS 单字节接收缓冲区（USART3 中断填入） */
extern uint8_t gps_rx_byte;
/** 全局 GPS 模块实例 */
extern gps_module gm;