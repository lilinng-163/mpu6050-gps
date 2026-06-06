/**
 * @file    task_cmt.cpp
 * @brief   通信任务 —— 每秒通过 USART2 发送 JSON 格式的姿态+GPS 数据（给 ESP32）
 * @date    2026-06-06
 *
 * @note    发送格式:
 *          {"gps":{"latitude": xx,"longitude": xx},"angels":{"roll": xx,"pitch": xx,"yaw": xx}}
 *
 * FreeRTOS 上下文: 栈 512 words / 优先级 5 / 周期 1000ms
 */

#include <string>
#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "uart_cb.hpp"
#include "task_cmt.hpp"
#include "task_heart.hpp"

using std::string;

// ======================== 模块级静态变量 ========================

/** 心跳寄存器指针 */
static volatile uint32_t *s_heart = NULL;
/** 姿态数据指针（由 MPU6050 计算任务写入） */
static three_angels *s_angels = NULL;
/** 保护姿态数据的互斥锁 */
static SemaphoreHandle_t s_angels_mutex = NULL;

// ======================== 任务配置常量 ========================

const static uint16_t cmt_task_stack_size = 512;
const static UBaseType_t cmt_task_priprity = 5;

// ======================== 任务函数 ========================

/**
 * @brief 通信任务 —— 组装 JSON 并发送到 USART2 (ESP32)
 */
static int cmt_task(void *pvParamters)
{
    vTaskDelay(1000);   // 等待其他任务先初始化
    static string send_buffer;
    while(1)
    {
        // 临界区: 读取姿态数据
        xSemaphoreTake(s_angels_mutex, portMAX_DELAY);
        float roll = s_angels->roll;
        float pitch = s_angels->pitch;
        float yaw = s_angels->yaw;
        xSemaphoreGive(s_angels_mutex);

        // 读取 GPS 数据
        static gps_data_t gps_xy;
        gm.get_data(gps_xy);

        // 组装 JSON
        send_buffer.resize(256);
        int len = snprintf(send_buffer.data(), send_buffer.size(),
            "{\"gps\":{\"latitude\": %f,\"longitude\": %f},"
            "\"angels\":{\"roll\": %f,\"pitch\": %f,\"yaw\": %f}}\r\n",
            gps_xy.latitude, gps_xy.longitude, roll, pitch, yaw);
        if(len > (int)send_buffer.size())
        {
            send_buffer.resize(len);
        }
        else if(len > 0)
        {
            // 通过 USART2 发送
            HAL_UART_Transmit(&huart2, (uint8_t *)send_buffer.data(), send_buffer.size(), HAL_MAX_DELAY);
        }
        else
        {
            break;
        }
        *s_heart |= CMT_TASK_STATUS;
        vTaskDelay(1000);
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建通信任务
 * @param heart         心跳寄存器引用
 * @param angels        姿态数据引用
 * @param angels_mutex  保护姿态数据的互斥锁（NULL 则返回 -1）
 * @return 0=成功, -1=参数无效
 */
int create_task_cmt(volatile uint32_t &heart,
                    three_angels &angels,
                    SemaphoreHandle_t angels_mutex)
{
    if(!angels_mutex) return -1;
    s_heart = &heart;
    s_angels = &angels;
    s_angels_mutex = angels_mutex;

    return xTaskCreate(
        (TaskFunction_t)cmt_task,
        (const char *)"cmt_task",
        cmt_task_stack_size,
        (void *)NULL,
        cmt_task_priprity,
        NULL
    );
}
