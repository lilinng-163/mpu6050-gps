#include <string>
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
#include "oled_096.hpp"

using std::string;

/*---------------------------------mpu6050任务相关---------------------------------*/

//舵机等待计算任务完成的信号量句柄
static QueueHandle_t servo_task_binary_handle;
//计算任务等待采集任务完成的信号量句柄
static QueueHandle_t mpu6050_task_binary_handle;

//存储原始mpu6050数据的结构体
static mpu6050_data_t mpu6050_data;

//存储roll pitch yaw
static three_angels angels;

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
            
        }
    }

    return 0;
}

const static uint16_t mpu6050_task_calculate_data_stack_size = 512;
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

            angels.roll = roll_deg;
            angels.pitch = pitch_deg;
            angels.yaw = yaw_deg;

            printf("%f %f %f\r\n", roll_deg, pitch_deg, yaw_deg);

            auto free = uxTaskGetStackHighWaterMark(NULL);
            printf("mpu6050_task_calculate_data free: %ld\r\n", free);

            //释放信号量给舵机
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
        auto free = uxTaskGetStackHighWaterMark(NULL);
        printf("mpu6050_task_collect_data free: %ld\r\n", free);
        xSemaphoreGive(mpu6050_task_binary_handle);
        vTaskDelay(100);
    }
    return 0;
}

/*---------------------------------oled任务相关---------------------------------*/
const static uint16_t oled_task_stack_size = 512;
const static UBaseType_t oled_task_priority = 2;
static TaskHandle_t oled_task_handle;

static int oled_task(void *pvParameters)
{
    static oled o1(GPIOB, GPIO_PIN_8, GPIO_PIN_9);
    o1.clear();
    while(1)
    {
        o1.clear();
        o1.show_num(angels.roll, 0, 0);
        o1.show_num(angels.pitch, 0, 16);
        o1.show_num(angels.yaw, 0, 32);
        o1.refresh();

        auto free = uxTaskGetStackHighWaterMark(NULL);
        printf("oled_task free: %ld\r\n", free);

        vTaskDelay(200);
    }
    return 0;
}
/*---------------------------------启动任务相关---------------------------------*/

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
    res = xTaskCreate((TaskFunction_t)oled_task,
                        (const char *)"oled_task",
                        oled_task_stack_size,
                        (void *)NULL,
                        oled_task_priority,
                        &oled_task_handle);
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
    int sum = oled_task_stack_size + servo_task_control_stack_size + mpu6050_task_calculate_data_stack_size + mpu6050_task_collect_data_stack_size;
    printf("total task memory: %dbytes\r\n", sum * 4);
    vTaskStartScheduler();
    return res;
}