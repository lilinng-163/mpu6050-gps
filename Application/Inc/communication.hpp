/**
 * @file    communication.hpp
 * @brief   通信缓冲区声明 —— USART2 与 ESP32 通信的发送/接收缓冲区
 * @date    2026-06-06
 */

#pragma once

#include <etl/vector.h>

using namespace etl;

/** USART2 发送缓冲区 (STM32 → ESP32) */
extern vector<char, 1024>cmt_tx_buf;
/** USART2 接收缓冲区 (ESP32 → STM32) */
extern vector<char, 1024>cmt_rx_buf;