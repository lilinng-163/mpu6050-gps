/**
 * @file    communication.cpp
 * @brief   通信缓冲区定义 —— USART2 与 ESP32 通信所用的发送/接收缓冲区
 * @date    2026-06-06
 */

#include <string>
#include <cstdint>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "usart.h"

using namespace etl;

/** USART2 发送缓冲区 (STM32 → ESP32) */
vector<char, 1024>cmt_tx_buf;
/** USART2 接收缓冲区 (ESP32 → STM32) */
vector<char, 1024>cmt_rx_buf;
