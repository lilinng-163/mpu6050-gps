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
#include "oled_096.hpp"
#include "led.hpp"
#include "tim6_get.hpp"
#include "uart_cb.hpp"
#include "task_heart.hpp"

using namespace etl;
using std::string;

//全局心跳维护
static volatile uint32_t heart = {0};

/*---------------------------------看门狗相关---------------------------------*/
const static uint16_t iwdg_task_stack_size = 256;
const static UBaseType_t iwdg_task_priority = 1;
static TaskHandle_t iwdg_task_handle;

static int iwdg_task(void *pvParamters)
{
    while(1)
    {
        vTaskDelay(5000);
        printf("iwdg check heart=0x%02X\r\n", (unsigned int)heart);
        if((heart & 0xFF) == 0xFF)
        {
            HAL_IWDG_Refresh(&hiwdg);
            heart = 0x00;
            printf("dog feed\r\n");
        }
        else
        {
            printf("heart: 0x%02X miss: ", (unsigned int)heart);
            if(!(heart & CPU_STATS_TASK_STATUS))              printf("cpu_stats ");
            if(!(heart & LED_TASK_BLINK_STATUS))              printf("led ");
            if(!(heart & SERVO_TASK_CONTROL_STATUS))           printf("servo ");
            if(!(heart & MPU6050_TASK_CALCULATE_DATA_STATUS))  printf("mpu_calc ");
            if(!(heart & MPU6050_TASK_COLLECT_DATA_STATUS))    printf("mpu_collect ");
            if(!(heart & OLED_TASK_STATUS))                    printf("oled ");
            if(!(heart & GPS_TASK_STATUS))                     printf("gps ");
            if(!(heart & CMT_TASK_STATUS))                     printf("cmt ");
            printf("\r\n");
        }

        // auto free = uxTaskGetStackHighWaterMark(NULL);
        // printf("iwdg free: %ld\r\n", free);

    }
    return 0;
}


/*---------------------------------统计cpu相关---------------------------------*/

const static uint16_t cpu_stats_task_stack_size = 512;
const static UBaseType_t cpu_stats_task_priority = 2;
static TaskHandle_t cpu_stats_handle;

static int cpu_stats_task(void *pvParamters)
{
    static char buf[512];

    while(1)
    {
        memset(buf, 0, sizeof(buf));

        vTaskGetRunTimeStats(buf);
        printf("===========cpu_stats===========\r\n");
        printf("%s\r\n", buf);

        // auto free = uxTaskGetStackHighWaterMark(NULL);
        // printf("cpu_stats_task free: %ld\r\n", free);

        heart |= CPU_STATS_TASK_STATUS;

        vTaskDelay(1000);
    }

    return 0;
}


/*---------------------------------led任务相关---------------------------------*/

const static uint16_t led_task_blink_stack_size = 128;
const static UBaseType_t led_task_blink_priority = 1;
static TaskHandle_t led_task_blink_handle;

static int led_task_blink(void *pvParamters)
{
    static led l1(GPIOB, GPIO_PIN_6);
    while(1)
    {
        l1.on();
        vTaskDelay(500);
        l1.off();
        vTaskDelay(500);
        heart |= LED_TASK_BLINK_STATUS;
    }
    return 0;
}

/*---------------------------------mpu6050任务相关---------------------------------*/

//舵机等待计算任务完成的信号量句柄
static QueueHandle_t servo_task_binary_handle;
//计算任务等待采集任务完成的信号量句柄
static QueueHandle_t mpu6050_task_binary_handle;

//存储原始mpu6050数据的结构体
static mpu6050_data_t mpu6050_data;

//存储roll pitch yaw
static three_angels angels;
//互斥锁
static SemaphoreHandle_t angels_mutex;
// UART 打印互斥锁 (供 main.c 的 __io_putchar 使用)
SemaphoreHandle_t uart_mutex;

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
            heart |= SERVO_TASK_CONTROL_STATUS;
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

            //拿锁，赋值,放锁
            xSemaphoreTake(angels_mutex, portMAX_DELAY);
            angels.roll = roll_deg;
            angels.pitch = pitch_deg;
            angels.yaw = yaw_deg;
            xSemaphoreGive(angels_mutex);

            // printf("%f %f %f\r\n", roll_deg, pitch_deg, yaw_deg);

            // auto free = uxTaskGetStackHighWaterMark(NULL);
            // printf("mpu6050_task_calculate_data free: %ld\r\n", free);

            heart |= MPU6050_TASK_CALCULATE_DATA_STATUS;

            //释放信号量给舵机
            xSemaphoreGive(servo_task_binary_handle);
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

        // auto free = uxTaskGetStackHighWaterMark(NULL);
        // printf("mpu6050_task_collect_data free: %ld\r\n", free);

        xSemaphoreGive(mpu6050_task_binary_handle);

        heart |= MPU6050_TASK_COLLECT_DATA_STATUS;

        vTaskDelay(50);
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
    static etl::vector<float, 3> v1 = {0};
    o1.clear();
    while(1)
    {
        o1.clear();
        //上锁，赋值，开锁
        xSemaphoreTake(angels_mutex, portMAX_DELAY);
        v1[0] = angels.roll;
        v1[1] = angels.pitch;
        v1[2] = angels.yaw;
        xSemaphoreGive(angels_mutex);
        
        o1.show_num(v1[0], 0, 0);
        o1.show_num(v1[1], 0, 16);
        o1.show_num(v1[2], 0, 32);
        o1.refresh();

        // auto free = uxTaskGetStackHighWaterMark(NULL);
        // printf("oled_task free: %ld\r\n", free);

        heart |= OLED_TASK_STATUS;

        vTaskDelay(500);
    }
    return 0;
}

/*---------------------------------gps相关---------------------------------*/

const static uint16_t gps_task_stack_size = 256;
const static UBaseType_t gps_task_priority = 5;
static TaskHandle_t gps_task_handle;

static int gps_task(void *pvParamters)
{
    HAL_UART_Receive_IT(&huart3, &gps_rx_byte, 1);
    while(1)
    {
        string line;
        gm.dump_raw(line);
        if(!line.empty())
        {
            printf("%s\r\n", line.c_str());
        }
        else
        {
            printf("gps_task no data\r\n");
        }
        // auto free = uxTaskGetStackHighWaterMark(NULL);
        // printf("gps_task free: %ld\r\n", free);

        heart |= GPS_TASK_STATUS;

        vTaskDelay(1000);
    }
    return 0;
}

/*---------------------------------通信相关---------------------------------*/

/*
    {
        "gps":
        {
            "latitude": 0.000000,
            "longitude": 0.000000
        },
        "angels":
        {
            "roll": 144.177063,
            "pitch": 27.390097,
            "yaw": -3.429999
        }
    }
*/

const static uint16_t cmt_task_stack_size = 512;
const static UBaseType_t cmt_task_priprity = 5;
static TaskHandle_t cmt_task_handle;

//communication
static int cmt_task(void *pvParamters)
{
    vTaskDelay(1000);
    static string send_buffer;
    while(1)
    {
        xSemaphoreTake(angels_mutex, portMAX_DELAY);
        float roll = angels.roll;
        float pitch = angels.pitch;
        float yaw = angels.yaw;
        xSemaphoreGive(angels_mutex);

        static gps_data_t gps_xy;
        gm.get_data(gps_xy);

        send_buffer.resize(256);
        int len = snprintf(send_buffer.data(), send_buffer.size(), "{\"gps\":{\"latitude\": %f,\"longitude\": %f},"
                            "\"angels\":{\"roll\": %f,\"pitch\": %f,\"yaw\": %f}}\r\n", 
                            gps_xy.latitude, gps_xy.longitude, roll, pitch, yaw);
        if(len > (int)send_buffer.size())                            
        {
            send_buffer.resize(len);
            printf("len > size");
        }
        else if(len > 0)
        {
            auto res = HAL_UART_Transmit(&huart2, (uint8_t *)send_buffer.data(), send_buffer.size(), HAL_MAX_DELAY);
            if(res == HAL_ERROR)
            {
                printf("send error\r\n");
            }
            // printf("len > 0\r\n");
        }
        else
        {
            printf("snprintf return: %d\r\n", len);
            break;
        }

        // auto free = uxTaskGetStackHighWaterMark(NULL);
        // printf("cmt_task free: %ld\r\n", free); 

        heart |= CMT_TASK_STATUS;

        vTaskDelay(1000);
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
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    }

    res = xTaskCreate((TaskFunction_t)led_task_blink,
                        (const char *)"led_task_blink",
                        led_task_blink_stack_size,
                        (void *)NULL,
                        led_task_blink_priority,
                        &led_task_blink_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    
    }

    res = xTaskCreate((TaskFunction_t)cpu_stats_task,
                        (const char *)"cpu_stats_task",
                        cpu_stats_task_stack_size,
                        (void *)NULL,
                        cpu_stats_task_priority,
                        &cpu_stats_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    
    }

    res = xTaskCreate((TaskFunction_t)gps_task,
                        (const char *)"gps_task",
                        gps_task_stack_size,
                        (void *)NULL,
                        gps_task_priority,
                        &gps_task_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    
    }

    res = xTaskCreate((TaskFunction_t)cmt_task,
                        (const char *)"cmt_task",
                        cmt_task_stack_size,
                        (void *)NULL,
                        cmt_task_priprity,
                        &cmt_task_handle);
    if(res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return res;
    
    }
       
    res = xTaskCreate((TaskFunction_t)iwdg_task,
                        (const char *)"iwdg_task",
                        iwdg_task_stack_size,
                        (void *)NULL,
                        iwdg_task_priority,
                        &iwdg_task_handle);
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

    //创建互斥锁
    angels_mutex = xSemaphoreCreateMutex();   
    if(angels_mutex == NULL)
    {
        printf("angels_mutex create fail\r\n");
    }
    else
    {
        printf("angels_mutex create successfully\r\n");
    }

    // UART 打印互斥锁 (后面 printf 都会用这个锁)
    uart_mutex = xSemaphoreCreateMutex();
    if(uart_mutex == NULL)
    {
        printf("uart_mutex create fail\r\n");
    }
    else
    {
        printf("uart_mutex create successfully\r\n");
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