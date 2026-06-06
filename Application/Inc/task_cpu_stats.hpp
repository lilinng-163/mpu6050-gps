/**
 * @file    task_cpu_stats.hpp
 * @brief   CPU 统计任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>

/**
 * @brief 创建 CPU 使用率统计任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_cpu_stats(volatile uint32_t &heart);
