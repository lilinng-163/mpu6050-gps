/**
 * @file    task_cmt.hpp
 * @brief   通信任务接口 —— 每秒发送 JSON 格式姿态+GPS 数据到 ESP32
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/semphr.h>
#include "mpu6050.hpp"

/**
 * @brief 创建通信任务
 * @param heart         心跳寄存器引用
 * @param angels        姿态角数据引用
 * @param angels_mutex  姿态角互斥锁；为 NULL 时返回 -1
 * @return 0=成功, -1=参数无效
 */
int create_task_cmt(volatile uint32_t &heart,
                    three_angels &angels,
                    SemaphoreHandle_t angels_mutex);
