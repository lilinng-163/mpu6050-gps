/**
 * @file    task_oled.cpp
 * @brief   OLED 显示任务 —— 在 OLED 屏幕上实时显示 MPU6050 姿态角（Roll/Pitch/Yaw）
 * @author  (your name)
 * @date    2026-06-06
 *
 * @details
 * 本任务周期性地从共享的 three_angels 结构体中读取姿态数据（受互斥锁保护），
 * 并刷新到 SH1122 驱动的 256x64 OLED 显示屏上。
 *
 * 依赖:
 *   - oled_208  (Devices)        OLED 驱动类
 *   - task_heart.hpp             心跳状态位定义 (OLED_TASK_STATUS)
 *   - mpu6050.hpp                three_angels 结构体定义
 *
 * FreeRTOS 上下文:
 *   - 栈大小: 512 words
 *   - 优先级: 2
 *   - 刷新周期: 500 ms
 */

#include <cstdint>
#include <etl/vector.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "oled_208.hpp"
#include "task_oled.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

/** 指向心跳寄存器的指针（由 app 层传入） */
static volatile uint32_t *s_heart = NULL;
/** 指向共享姿态数据的指针（由 MPU6050 计算任务写入） */
static three_angels *s_angels = NULL;
/** 保护姿态数据的互斥锁句柄 */
static SemaphoreHandle_t s_angels_mutex = NULL;

// ======================== 任务配置常量 ========================

/** OLED 刷新任务的栈大小（单位: FreeRTOS words, 通常 4 字节/word） */
const static uint16_t oled_task_stack_size = 512;
/** OLED 刷新任务的 FreeRTOS 优先级 */
const static UBaseType_t oled_task_priority = 2;

// ======================== 任务函数 ========================

/**
 * @brief OLED 显示任务主循环
 *
 * 每个周期:
 *   1. 清屏
 *   2. 获取互斥锁，读取 Roll/Pitch/Yaw
 *   3. 释放互斥锁
 *   4. 将三个姿态角分别显示在 OLED 的 (0,0), (0,16), (0,32) 三行
 *   5. 刷新屏幕
 *   6. 设置心跳状态位，通知看门狗本任务存活
 *   7. 延时 500 ms
 *
 * @param pvParameters  未使用（NULL）
 * @return 0            FreeRTOS 任务退出码（本任务永不退出）
 */
static int oled_task(void *pvParameters)
{
    // oled_208 对象 —— 使用软件 I2C，SCL=PB8, SDA=PB9
    static oled_208 o1(GPIOB, GPIO_PIN_8, GPIO_PIN_9);
    // 临时数组: 依次存放 Roll, Pitch, Yaw
    static etl::vector<float, 3> v1 = {0};
    o1.clear();
    while(1)
    {
        o1.clear();
        // ---- 临界区：读取共享姿态数据 ----
        xSemaphoreTake(s_angels_mutex, portMAX_DELAY);
        v1[0] = s_angels->roll;
        v1[1] = s_angels->pitch;
        v1[2] = s_angels->yaw;
        xSemaphoreGive(s_angels_mutex);
        // ---- 临界区结束 ----

        // 将三个角度显示在 OLED 左列，每行间隔 16 像素
        o1.show_num(v1[0], 0, 0);   // Roll  在第 1 行
        o1.show_num(v1[1], 0, 16);  // Pitch 在第 2 行
        o1.show_num(v1[2], 0, 32);  // Yaw   在第 3 行
        o1.refresh();

        // 报告心跳：通知看门狗本任务正常运行
        *s_heart |= OLED_TASK_STATUS;
        vTaskDelay(500);
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建 OLED 显示任务
 *
 * @param heart         心跳寄存器引用
 * @param angels        共享姿态数据引用
 * @param angels_mutex  保护姿态数据的互斥锁句柄（NULL 则返回 -1）
 * @return int          0 = 成功，-1 = 参数无效，其他 = FreeRTOS 错误码
 */
int create_task_oled(volatile uint32_t &heart,
                     three_angels &angels,
                     SemaphoreHandle_t angels_mutex)
{
    if(!angels_mutex) return -1;
    s_heart = &heart;
    s_angels = &angels;
    s_angels_mutex = angels_mutex;

    // 创建 FreeRTOS 任务
    return xTaskCreate(
        (TaskFunction_t)oled_task,
        (const char *)"oled_task",
        oled_task_stack_size,
        (void *)NULL,
        oled_task_priority,
        NULL
    );
}
