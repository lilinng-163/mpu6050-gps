/**
 * @file    task_iwdg.hpp
 * @brief   软件看门狗任务接口
 * @date    2026-06-06
 */

#pragma once

#include <cstdint>

/**
 * @brief 创建软件看门狗任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_iwdg(volatile uint32_t &heart);
