/**
 * @file    task_gps.hpp
 * @brief   GPS 数据读取任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>

/**
 * @brief 创建 GPS 数据读取任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_gps(volatile uint32_t &heart);
