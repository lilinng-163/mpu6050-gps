/**
 * @file    task_iwdg.cpp
 * @brief   软件看门狗任务 —— 每 5 秒检查心跳寄存器，全位为 1 则喂狗，否则打印缺失任务
 * @date    2026-06-06
 *
 * @note    心跳寄存器 (s_heart) 的 bit 0~7 对应 8 个任务的存活状态。
 *          所有任务都在本周期内报告过 (0xFF) → 喂硬件 IWDG。
 *          有任务缺失 → 串口打印缺失列表，不喂狗 → 系统复位。
 *
 * FreeRTOS 上下文: 栈 256 words / 优先级 1 / 周期 5000ms
 */

#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "iwdg.h"
#include "task_iwdg.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

/** 心跳寄存器指针 */
static volatile uint32_t *s_heart = NULL;

// ======================== 任务配置常量 ========================

const static uint16_t iwdg_task_stack_size = 256;
const static UBaseType_t iwdg_task_priority = 1;

// ======================== 任务函数 ========================

/**
 * @brief 软件看门狗监控任务
 *
 * 每 5 秒检查一次心跳寄存器:
 *   - 0xFF → 所有任务存活 → 喂狗 (HAL_IWDG_Refresh) → 清零心跳
 *   - 否则 → 打印缺失的任务名 → 不喂狗 → 等待硬件复位
 */
static int iwdg_task(void *pvParamters)
{
    while(1)
    {
        vTaskDelay(5000);
        printf("iwdg check heart=0x%02X\r\n", (unsigned int)(*s_heart));
        if((*s_heart & 0xFF) == 0xFF)
        {
            // 所有任务存活 → 喂狗 + 清零心跳
            HAL_IWDG_Refresh(&hiwdg);
            *s_heart = 0x00;
            printf("dog feed\r\n");
        }
        else
        {
            // 缺失检测: 逐位检查并打印异常任务名
            printf("heart: 0x%02X miss: ", (unsigned int)(*s_heart));
            if(!(*s_heart & CPU_STATS_TASK_STATUS))              printf("cpu_stats ");
            if(!(*s_heart & LED_TASK_BLINK_STATUS))              printf("led ");
            if(!(*s_heart & SERVO_TASK_CONTROL_STATUS))           printf("servo ");
            if(!(*s_heart & MPU6050_TASK_CALCULATE_DATA_STATUS))  printf("mpu_calc ");
            if(!(*s_heart & MPU6050_TASK_COLLECT_DATA_STATUS))    printf("mpu_collect ");
            if(!(*s_heart & OLED_TASK_STATUS))                    printf("oled ");
            if(!(*s_heart & GPS_TASK_STATUS))                     printf("gps ");
            if(!(*s_heart & CMT_TASK_STATUS))                     printf("cmt ");
            printf("\r\n");
        }
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建软件看门狗任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_iwdg(volatile uint32_t &heart)
{
    s_heart = &heart;

    return xTaskCreate(
        (TaskFunction_t)iwdg_task,
        (const char *)"iwdg_task",
        iwdg_task_stack_size,
        (void *)NULL,
        iwdg_task_priority,
        NULL
    );
}
