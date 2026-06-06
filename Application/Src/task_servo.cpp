/**
 * @file    task_servo.cpp
 * @brief   舵机控制任务 —— 等待二值信号量，收到后执行舵机动作并上报心跳
 * @date    2026-06-06
 *
 * @note    舵机由 MPU6050 计算任务通过二值信号量触发，形成生产者-消费者模式。
 *          当前为占位实现，实际舵机控制逻辑待完善。
 *
 * FreeRTOS 上下文: 栈 512 words / 优先级 3 / 事件驱动（阻塞等待）
 */

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "servo_SG90.hpp"
#include "task_servo.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

/** 指向心跳寄存器的指针 */
static volatile uint32_t *s_heart = NULL;
/** 舵机触发信号量（由 MPU6050 计算任务 give） */
static QueueHandle_t s_servo_binary = NULL;

// ======================== 任务配置常量 ========================

const static uint16_t servo_task_control_stack_size = 512;
const static UBaseType_t servo_task_control_priority = 3;

// ======================== 任务函数 ========================

/**
 * @brief 舵机控制任务 —— 阻塞等待二值信号量，收到后更新心跳
 */
static int servo_task_control(void *pvParamters)
{
    // 舵机对象: TIM2 CH1, 初始占空比 0
    static servo_sg90 s1(&htim2, TIM_CHANNEL_1, 0);
    while(1)
    {
        // 阻塞等待触发信号
        if(xSemaphoreTake(s_servo_binary, portMAX_DELAY))
            *s_heart |= SERVO_TASK_CONTROL_STATUS;
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建舵机控制任务
 * @param heart   心跳寄存器引用
 * @param binary  二值信号量句柄（NULL 则返回 -1）
 * @return 0=成功, -1=参数无效
 */
int create_task_servo(volatile uint32_t &heart, QueueHandle_t binary)
{
    if(!binary) return -1;
    s_heart = &heart;
    s_servo_binary = binary;

    return xTaskCreate(
        (TaskFunction_t)servo_task_control,
        (const char *)"servo_task_control",
        servo_task_control_stack_size,
        (void *)NULL,
        servo_task_control_priority,
        NULL
    );
}
