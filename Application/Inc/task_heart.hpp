/**
 * @file    task_heart.hpp
 * @brief   心跳状态位定义 —— 每个任务对应一个 bit，用于软件看门狗检测
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>

#define CPU_STATS_TASK_STATUS                       (1 << 0)  /**< CPU 统计任务存活 */
#define LED_TASK_BLINK_STATUS                       (1 << 1)  /**< LED 闪烁任务存活 */
#define SERVO_TASK_CONTROL_STATUS                   (1 << 2)  /**< 舵机控制任务存活 */
#define MPU6050_TASK_CALCULATE_DATA_STATUS          (1 << 3)  /**< 姿态解算任务存活 */
#define MPU6050_TASK_COLLECT_DATA_STATUS            (1 << 4)  /**< 传感器采集任务存活 */
#define OLED_TASK_STATUS                            (1 << 5)  /**< OLED 显示任务存活 */
#define GPS_TASK_STATUS                             (1 << 6)  /**< GPS 读取任务存活 */
#define CMT_TASK_STATUS                             (1 << 7)  /**< 通信任务存活 */
