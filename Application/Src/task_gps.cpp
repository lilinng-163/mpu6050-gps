/**
 * @file    task_gps.cpp
 * @brief   GPS 数据读取任务 —— 每秒从 gps_module 缓冲区中取出原始 NMEA 语句并打印
 * @date    2026-06-06
 *
 * @note    GPS 原始字节由 USART3 中断回调 (uart_cb.cpp) 写入环形缓冲区，
 *          本任务负责定期消费。
 *
 * FreeRTOS 上下文: 栈 256 words / 优先级 5 / 周期 1000ms
 */

#include <string>
#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "uart_cb.hpp"
#include "task_gps.hpp"
#include "task_heart.hpp"

using std::string;

// ======================== 模块级静态变量 ========================

/** 指向心跳寄存器的指针 */
static volatile uint32_t *s_heart = NULL;

// ======================== 任务配置常量 ========================

const static uint16_t gps_task_stack_size = 256;
const static UBaseType_t gps_task_priority = 5;

// ======================== 任务函数 ========================

/**
 * @brief GPS 数据消费任务 —— 每秒 dump 一次原始数据
 */
static int gps_task(void *pvParamters)
{
    // 启动 USART3 中断接收（单字节循环）
    HAL_UART_Receive_IT(&huart3, &gps_rx_byte, 1);
    while(1)
    {
        string line;
        gm.dump_raw(line);       // 从环形缓冲区取出所有字节
        if(!line.empty())
            printf("%s\r\n", line.c_str());
        else
            printf("gps_task no data\r\n");
        *s_heart |= GPS_TASK_STATUS;
        vTaskDelay(1000);
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建 GPS 数据读取任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_gps(volatile uint32_t &heart)
{
    s_heart = &heart;

    return xTaskCreate(
        (TaskFunction_t)gps_task,
        (const char *)"gps_task",
        gps_task_stack_size,
        (void *)NULL,
        gps_task_priority,
        NULL
    );
}
