/**
 * @file    task_mpu6050_collect.hpp
 * @brief   MPU6050 数据采集任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/semphr.h>
#include "mpu6050.hpp"

/**
 * @brief 创建 MPU6050 数据采集任务
 * @param heart   心跳寄存器引用
 * @param data    六轴原始数据缓冲区引用
 * @param binary  二值信号量（通知计算任务）；为 NULL 时返回 -1
 * @return 0=成功, -1=参数无效
 */
int create_task_mpu6050_collect(volatile uint32_t &heart,
                                mpu6050_data_t &data,
                                QueueHandle_t binary);
