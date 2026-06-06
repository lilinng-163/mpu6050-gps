/**
 * @file    app.cpp
 * @brief   应用层主入口 —— 创建所有 FreeRTOS 同步原语和任务，启动调度器
 * @date    2026-06-06
 *
 * @details
 * 启动流程:
 *   1. 创建二值信号量 (mpu6050 采集→计算, 舵机触发)
 *   2. 创建互斥锁 (姿态数据保护, UART 打印保护)
 *   3. 创建 start_task → 依次创建所有子任务
 *   4. 启动 FreeRTOS 调度器
 *
 * 任务依赖关系:
 *   CPU Stats ────────────────────────────── 独立
 *   LED Blink ────────────────────────────── 独立
 *   IWDG Watchdog ────────────────────────── 独立（监控所有心跳）
 *   GPS Task ─────────────────────────────── USART3 中断 → 环形缓冲
 *   MPU6050 Collect ──(信号量)──→ Calculate ──(互斥锁)──→ OLED
 *                                               ├──→ CMT (USART2)
 *                                               └──(信号量)──→ Servo
 */

#include <string>
#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <FreeRTOS/semphr.h>
#include <FreeRTOS/queue.h>
#include <etl/algorithm.h>
#include <etl/vector.h>
#include <etl/circular_buffer.h>
#include <arm_math.h>
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "iwdg.h"
#include "app.hpp"
#include "mpu6050.hpp"
#include "servo_SG90.hpp"
#include "oled_208.hpp"
#include "led.hpp"
#include "tim6_get.hpp"
#include "uart_cb.hpp"
#include "task_heart.hpp"
#include "task_iwdg.hpp"
#include "task_cpu_stats.hpp"
#include "task_led.hpp"
#include "task_servo.hpp"
#include "task_mpu6050_collect.hpp"
#include "task_mpu6050_calculate.hpp"
#include "task_oled.hpp"
#include "task_gps.hpp"
#include "task_cmt.hpp"

using namespace etl;
using std::string;

// ======================== 全局变量 ========================

/** 全局心跳寄存器: bit0~7 对应 8 个任务的存活状态 */
static volatile uint32_t heart = {0};

// ======================== MPU6050 / 姿态 / 舵机 相关 ========================

/** 舵机触发信号量 (计算任务 give → 舵机任务 take) */
static QueueHandle_t servo_task_binary_handle;
/** MPU6050 采集完成信号量 (采集任务 give → 计算任务 take) */
static QueueHandle_t mpu6050_task_binary_handle;

/** 六轴原始数据（采集任务写入，计算任务读取） */
static mpu6050_data_t mpu6050_data;

/** 姿态角输出（计算任务写入，OLED/通信任务读取） */
static three_angels angels;
/** 保护姿态角的互斥锁 */
static SemaphoreHandle_t angels_mutex;
/** UART 打印互斥锁（避免多任务 printf 交织） */
SemaphoreHandle_t uart_mutex;

// ======================== 启动任务 ========================

const static uint16_t start_task_stack_size = 128;
const static UBaseType_t start_task_priority = 1;
static TaskHandle_t start_task_handle;

/**
 * @brief 启动任务 —— 在临界区中依次创建所有子任务
 *
 * 任一子任务创建失败（内存不足）则立即返回错误码。
 * 全部创建成功后删除自身。
 */
static int start_task(void *pvParamters)
{
    taskENTER_CRITICAL();

    // ---- MPU6050 采集任务 ----
    auto res = create_task_mpu6050_collect(heart, mpu6050_data, mpu6050_task_binary_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- MPU6050 姿态解算任务 ----
    res = create_task_mpu6050_calculate(heart, mpu6050_data, mpu6050_task_binary_handle,
                                        angels, angels_mutex, servo_task_binary_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- 舵机控制任务 ----
    res = create_task_servo(heart, servo_task_binary_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- OLED 显示任务 ----
    res = create_task_oled(heart, angels, angels_mutex);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- LED 闪烁任务 ----
    res = create_task_led(heart);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- CPU 统计任务 ----
    res = create_task_cpu_stats(heart);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- GPS 数据读取任务 ----
    res = create_task_gps(heart);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- 通信任务 (USART2 → ESP32) ----
    res = create_task_cmt(heart, angels, angels_mutex);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    // ---- 软件看门狗任务 ----
    res = create_task_iwdg(heart);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) { return res; }

    taskEXIT_CRITICAL();

    // 启动任务完成使命，删除自身
    vTaskDelete(NULL);
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 启动 FreeRTOS —— 创建同步原语 → 创建启动任务 → 启动调度器
 * @return xTaskCreate 的返回值
 *
 * @note  此函数由 main.c 在硬件初始化完成后调用。
 *        调用 vTaskStartScheduler() 后不会返回（除非内存不足）。
 */
int start_freertos(void)
{
    // ---- 创建二值信号量 ----

    mpu6050_task_binary_handle = xSemaphoreCreateBinary();
    if(mpu6050_task_binary_handle == NULL)
    {
        printf("mpu6050_task_collect_data create semaphor binary fail\r\n");
    }
    else
    {
        printf("mpu6050_task_collect_data create semaphor binary successfully\r\n");
    }

    servo_task_binary_handle = xSemaphoreCreateBinary();
    if(servo_task_binary_handle == NULL)
    {
        printf("servo_task_control create semaphor binary fail\r\n");
    }
    else
    {
        printf("servo_task_control create semaphor binary successfully\r\n");
    }

    // ---- 创建互斥锁 ----

    angels_mutex = xSemaphoreCreateMutex();
    if(angels_mutex == NULL)
    {
        printf("angels_mutex create fail\r\n");
    }
    else
    {
        printf("angels_mutex create successfully\r\n");
    }

    // UART 打印互斥锁 (避免多任务 printf 输出交织)
    uart_mutex = xSemaphoreCreateMutex();
    if(uart_mutex == NULL)
    {
        printf("uart_mutex create fail\r\n");
    }
    else
    {
        printf("uart_mutex create successfully\r\n");
    }

    // ---- 创建启动任务 ----
    int res = xTaskCreate((TaskFunction_t)start_task,
                            (const char *)"start_task",
                            start_task_stack_size,
                            (void *)NULL,
                            start_task_priority,
                            &start_task_handle);

    // ---- 启动调度器（此调用不会返回） ----
    vTaskStartScheduler();
    return res;
}