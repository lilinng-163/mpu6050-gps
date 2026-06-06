/**
 * @file    task_led.hpp
 * @brief   LED 闪烁任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>

/**
 * @brief 创建 LED 闪烁任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_led(volatile uint32_t &heart);
