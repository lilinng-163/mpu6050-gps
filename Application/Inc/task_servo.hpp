/**
 * @file    task_servo.hpp
 * @brief   舵机控制任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/semphr.h>

/**
 * @brief 创建舵机控制任务
 * @param heart   心跳寄存器引用
 * @param binary  二值信号量（由计算任务触发）；为 NULL 时返回 -1
 * @return 0=成功, -1=参数无效
 */
int create_task_servo(volatile uint32_t &heart, QueueHandle_t binary);
