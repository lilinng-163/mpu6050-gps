/**
 * @file    task_cpu_stats.cpp
 * @brief   CPU 使用率统计任务 —— 每秒打印各 FreeRTOS 任务的运行时间占比
 * @date    2026-06-06
 *
 * @note    依赖 FreeRTOS run-time stats 功能，需要:
 *          - configGENERATE_RUN_TIME_STATS = 1
 *          - vConfigureTimerForRunTimeStats() 和 ulGetRunTimeCounterValue() (在 tim6_get.cpp)
 *
 * FreeRTOS 上下文: 栈 512 words / 优先级 2 / 周期 1000ms
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "task_cpu_stats.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

static volatile uint32_t *s_heart = NULL;

// ======================== 任务配置常量 ========================

const static uint16_t cpu_stats_task_stack_size = 512;
const static UBaseType_t cpu_stats_task_priority = 2;

// ======================== 任务函数 ========================

/**
 * @brief CPU 统计任务 —— 调用 vTaskGetRunTimeStats 并打印
 */
static int cpu_stats_task(void *pvParamters)
{
    static char buf[512];   // FreeRTOS 运行时统计字符串缓冲区
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        vTaskGetRunTimeStats(buf);
        printf("===========cpu_stats===========\r\n");
        printf("%s\r\n", buf);
        *s_heart |= CPU_STATS_TASK_STATUS;
        vTaskDelay(1000);
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建 CPU 统计任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_cpu_stats(volatile uint32_t &heart)
{
    s_heart = &heart;

    return xTaskCreate(
        (TaskFunction_t)cpu_stats_task,
        (const char *)"cpu_stats_task",
        cpu_stats_task_stack_size,
        (void *)NULL,
        cpu_stats_task_priority,
        NULL
    );
}
