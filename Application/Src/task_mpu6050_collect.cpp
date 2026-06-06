/**
 * @file    task_mpu6050_collect.cpp
 * @brief   MPU6050 数据采集任务 —— 50ms 定时读取六轴原始数据并通知计算任务
 * @date    2026-06-06
 *
 * @note    采集→计算 通过二值信号量同步，形成流水线。
 *          I2C 使用软件模拟 (PC0=SCL, PC1=SDA)。
 *
 * FreeRTOS 上下文: 栈 512 words / 优先级 2 / 周期 50ms
 */

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "mpu6050.hpp"
#include "task_mpu6050_collect.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

/** 心跳寄存器指针 */
static volatile uint32_t *s_heart = NULL;
/** 六轴原始数据缓冲区（与计算任务共享） */
static mpu6050_data_t *s_data = NULL;
/** 通知计算任务的二值信号量 */
static QueueHandle_t s_mpu6050_binary = NULL;

// ======================== 任务配置常量 ========================

const static uint16_t task_stack_size = 512;
const static UBaseType_t task_priority = 2;

// ======================== 任务函数 ========================

/**
 * @brief MPU6050 采集任务 —— 每 50ms 读一次传感器 → give 信号量 → 延时
 */
static int mpu6050_task_collect_data(void *pvParamters)
{
    // MPU6050 对象: 软件 I2C, PC0=SCL, PC1=SDA
    static mpu6050 m1(GPIOC, GPIO_PIN_0, GPIO_PIN_1);
    while(1)
    {
        m1.get_data(*s_data);                       // 读取 14 字节原始数据
        xSemaphoreGive(s_mpu6050_binary);           // 通知计算任务
        *s_heart |= MPU6050_TASK_COLLECT_DATA_STATUS;
        vTaskDelay(50);
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建 MPU6050 数据采集任务
 * @param heart   心跳寄存器引用
 * @param data    六轴原始数据缓冲区引用
 * @param binary  二值信号量（NULL 则返回 -1）
 * @return 0=成功, -1=参数无效
 */
int create_task_mpu6050_collect(volatile uint32_t &heart,
                                mpu6050_data_t &data,
                                QueueHandle_t binary)
{
    if(!binary) return -1;
    s_heart = &heart;
    s_data = &data;
    s_mpu6050_binary = binary;

    return xTaskCreate(
        (TaskFunction_t)mpu6050_task_collect_data,
        (const char *)"mpu6050_task_collect_data",
        task_stack_size,
        (void *)NULL,
        task_priority,
        NULL
    );
}
