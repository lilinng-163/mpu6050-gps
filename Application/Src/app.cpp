#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "stm32f1xx_hal.h"
#include "app.hpp"

const static uint16_t start_task_stack_size = 128;
const static UBaseType_t start_task_priority = 1;
static TaskHandle_t start_task_handle;

static int start_task(void *pvParamters)
{
    taskEXIT_CRITICAL();
    // mpu6050任务
    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
    return 0;
}

int start_freertos(void)
{
    int res = xTaskCreate((TaskFunction_t)start_task,
                            (const char *)"start_task",
                            start_task_stack_size,
                            (void *)NULL,
                            start_task_priority,
                            &start_task_handle);

    return res;
}