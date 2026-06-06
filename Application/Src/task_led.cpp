/**
 * @file    task_led.cpp
 * @brief   LED 闪烁任务 —— 以 500ms 周期闪烁板载 LED (PB6)，兼作心跳指示
 * @date    2026-06-06
 *
 * FreeRTOS 上下文: 栈 128 words / 优先级 1 / 周期 500ms
 */

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "led.hpp"
#include "task_led.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

/** 指向心跳寄存器的指针（由 app 层传入） */
static volatile uint32_t *s_heart = NULL;

// ======================== 任务配置常量 ========================

/** 栈大小（FreeRTOS words） */
const static uint16_t led_task_blink_stack_size = 128;
/** FreeRTOS 优先级 */
const static UBaseType_t led_task_blink_priority = 1;

// ======================== 任务函数 ========================

/**
 * @brief LED 闪烁任务 —— 亮 500ms → 灭 500ms 循环
 */
static int led_task_blink(void *pvParamters)
{
    // LED 对象: PB6, 低电平点亮
    static led l1(GPIOB, GPIO_PIN_6);
    while(1)
    {
        l1.on();
        vTaskDelay(500);
        l1.off();
        vTaskDelay(500);
        // 报告心跳
        *s_heart |= LED_TASK_BLINK_STATUS;
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建 LED 闪烁任务
 * @param heart  心跳寄存器引用
 * @return 0=成功
 */
int create_task_led(volatile uint32_t &heart)
{
    s_heart = &heart;

    return xTaskCreate(
        (TaskFunction_t)led_task_blink,
        (const char *)"led_task_blink",
        led_task_blink_stack_size,
        (void *)NULL,
        led_task_blink_priority,
        NULL
    );
}
