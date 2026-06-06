/**
 * @file    task_oled.hpp
 * @brief   OLED 显示任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/semphr.h>
#include "mpu6050.hpp"

/**
 * @brief 创建 OLED 显示任务
 * @param heart         心跳寄存器引用
 * @param angels        姿态角数据引用
 * @param angels_mutex  姿态角互斥锁；为 NULL 时返回 -1
 * @return 0=成功, -1=参数无效
 */
int create_task_oled(volatile uint32_t &heart,
                     three_angels &angels,
                     SemaphoreHandle_t angels_mutex);
