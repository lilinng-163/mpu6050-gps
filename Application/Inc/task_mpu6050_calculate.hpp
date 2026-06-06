/**
 * @file    task_mpu6050_calculate.hpp
 * @brief   MPU6050 姿态解算任务接口 —— 互补滤波
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/semphr.h>
#include "mpu6050.hpp"

/**
 * @brief 创建 MPU6050 姿态解算任务
 * @param heart            心跳寄存器引用
 * @param data             六轴原始数据引用
 * @param mpu6050_binary   采集完成信号量；为 NULL 时返回 -1
 * @param angels           输出姿态角引用
 * @param angels_mutex     姿态角互斥锁；为 NULL 时返回 -1
 * @param servo_binary     舵机触发信号量；为 NULL 时返回 -1
 * @return 0=成功, -1=参数无效
 */
int create_task_mpu6050_calculate(volatile uint32_t &heart,
                                  mpu6050_data_t &data,
                                  QueueHandle_t mpu6050_binary,
                                  three_angels &angels,
                                  SemaphoreHandle_t angels_mutex,
                                  QueueHandle_t servo_binary);
