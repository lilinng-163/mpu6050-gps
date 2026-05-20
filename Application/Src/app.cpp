#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "app.hpp"
#include "mpu6050.hpp"

static mpu6050_data_t mpu6050_data;
const static uint16_t mpu6050_task_collect_data_stack_size = 512;
const static UBaseType_t mpu6050_task_collect_data_priority = 4;
static TaskHandle_t mpu6050_task_collect_data_handle;

static int mpu6050_task_collect_data(void *pvParamters)
{
    static mpu6050 m1(GPIOC, GPIO_PIN_0, GPIO_PIN_1);
    while(1)
    {
        m1.get_data(mpu6050_data);
        printf("AX: %d\r\n",mpu6050_data.accel_x);
        vTaskDelay(200);
    }
    return 0;
}

const static uint16_t start_task_stack_size = 128;
const static UBaseType_t start_task_priority = 1;
static TaskHandle_t start_task_handle;

static int start_task(void *pvParamters)
{
    taskENTER_CRITICAL();
    // mpu6050任务
    auto res = xTaskCreate((TaskFunction_t)mpu6050_task_collect_data,
                            (const char *)"mpu6050_task_collect_data",
                            mpu6050_task_collect_data_stack_size,
                            (void *)NULL,
                            mpu6050_task_collect_data_priority,
                            &mpu6050_task_collect_data_handle);
    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
    return res;
}

int start_freertos(void)
{
    int res = xTaskCreate((TaskFunction_t)start_task,
                            (const char *)"start_task",
                            start_task_stack_size,
                            (void *)NULL,
                            start_task_priority,
                            &start_task_handle);
    vTaskStartScheduler();
    return res;
}