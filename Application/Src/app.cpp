#include <cstdio>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <FreeRTOS/projdefs.h>
#include <FreeRTOS/semphr.h>
#include <FreeRTOS/queue.h>
#include <arm_math.h>
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "app.hpp"
#include "mpu6050.hpp"
#include "servo_SG90.hpp"

static QueueHandle_t servo_task_binary_handle;

static mpu6050_data_t mpu6050_data;
static QueueHandle_t mpu6050_task_binary_handle;

const static uint16_t servo_task_control_stack_size = 512;
const static UBaseType_t servo_task_control_priority = 3;
static TaskHandle_t servo_task_control_handle;

static int servo_task_control(void *pvParamters)
{
    static servo_sg90 s1(&htim2, TIM_CHANNEL_1, 0);
    while(1)
    {
        if(xSemaphoreTake(servo_task_binary_handle, portMAX_DELAY))
        {
            for(auto i = 0; i < 100; i++)
            {
                s1.set_duty(i);
                printf("servo duty: %d\r\n", i);
            }
        }
    }

    return 0;
}

const static uint16_t mpu6050_task_calculate_data_stack_size = 1024;
const static UBaseType_t mpu6050_task_calculate_data_priority = 4;
static TaskHandle_t mpu6050_task_calculate_data_handle;

static int mpu6050_task_calculate_data(void *pvParamters)
{
    // 存储上一次的滤波值
    static float filter_roll  = 0.0f;
    static float filter_pitch = 0.0f;
    static float filter_yaw   = 0.0f;
    static TickType_t last_tick = 0;
    const float alpha = 0.98f;

    while(1)
    {
        if(xSemaphoreTake(mpu6050_task_binary_handle, portMAX_DELAY))
        {
            // ── 计算 dt（两次更新之间的实际时间）──
            TickType_t now = xTaskGetTickCount();
            if (last_tick == 0) { last_tick = now; continue; }
            float dt = (now - last_tick) * portTICK_PERIOD_MS / 1000.0f;
            last_tick = now;

            float ax = mpu6050_data.accel_x / 2048.0f;
            float ay = mpu6050_data.accel_y / 2048.0f;
            float az = mpu6050_data.accel_z / 2048.0f;

            float gx = (mpu6050_data.gyro_x / 16.4f) * PI / 180.0f;   // rad/s
            float gy = (mpu6050_data.gyro_y / 16.4f) * PI / 180.0f;
            float gz = (mpu6050_data.gyro_z / 16.4f) * PI / 180.0f;

            // ── 加速度计计算参考角度 ──
            float accel_roll, accel_pitch, tmp;
            //roll = arctan(az / ay)
            arm_atan2_f32(ay, az, &accel_roll);

            //pitch = arctan(-ax / (sqrt(ay^2 + az^2)))
            //开方
            arm_sqrt_f32(ay * ay + az * az, &tmp);
            arm_atan2_f32(-ax, tmp, &accel_pitch);

            // ── 互补滤波 ──
            filter_roll  = alpha * (filter_roll  + gx * dt) + (1.0f - alpha) * accel_roll;
            filter_pitch = alpha * (filter_pitch + gy * dt) + (1.0f - alpha) * accel_pitch;
            filter_yaw  += gz * dt;

            float roll_deg  = filter_roll  * 180.0f / PI;
            float pitch_deg = filter_pitch * 180.0f / PI;
            float yaw_deg   = filter_yaw   * 180.0f / PI;

            printf("%f %f %f\r\n", roll_deg, pitch_deg, yaw_deg);
            auto free = uxTaskGetStackHighWaterMark(NULL);
            printf("free: %ld\r\n", free);

            xSemaphoreGive(servo_task_binary_handle);

            vTaskDelay(200);
        }
    }
    return 0;
}

const static uint16_t mpu6050_task_collect_data_stack_size = 512;
const static UBaseType_t mpu6050_task_collect_data_priority = 2;
static TaskHandle_t mpu6050_task_collect_data_handle;

static int mpu6050_task_collect_data(void *pvParamters)
{
    static mpu6050 m1(GPIOC, GPIO_PIN_0, GPIO_PIN_1);
    while(1)
    {
        m1.get_data(mpu6050_data);
        printf("AX: %d\r\n",mpu6050_data.accel_x);
        xSemaphoreGive(mpu6050_task_binary_handle);
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
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    }
    res = xTaskCreate((TaskFunction_t)mpu6050_task_calculate_data,
                        (const char *)"mpu6050_task_calculate_data",
                        mpu6050_task_calculate_data_stack_size,
                        (void *)NULL,
                        mpu6050_task_calculate_data_priority,
                        &mpu6050_task_calculate_data_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    }
    res = xTaskCreate((TaskFunction_t)servo_task_control,
                        (const char *)"servo_task_control",
                        servo_task_control_stack_size,
                        (void *)NULL,
                        servo_task_control_priority,
                        &servo_task_control_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    }
    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
    return 0;
}



int start_freertos(void)
{
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
    int res = xTaskCreate((TaskFunction_t)start_task,
                            (const char *)"start_task",
                            start_task_stack_size,
                            (void *)NULL,
                            start_task_priority,
                            &start_task_handle);
    vTaskStartScheduler();
    return res;
}