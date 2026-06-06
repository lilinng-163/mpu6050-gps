/**
 * @file    task_mpu6050_calculate.cpp
 * @brief   MPU6050 姿态解算任务 —— 互补滤波融合加速度计+陀螺仪，输出 Roll/Pitch/Yaw
 * @date    2026-06-06
 *
 * @details
 * 算法: 互补滤波 (Complementary Filter), α = 0.98
 *   - 加速度计 → 静态 Roll/Pitch (atan2)
 *   - 陀螺仪   → 角速度积分
 *   - 融合: angle = α × (angle + gyro×dt) + (1-α) × accel_angle
 *
 * 数据流: 采集任务(give) → 本任务(take) → OLED/通信/舵机
 *
 * FreeRTOS 上下文: 栈 1024 words / 优先级 4 / 事件驱动
 */

#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <arm_math.h>
#include "task_mpu6050_calculate.hpp"
#include "task_heart.hpp"

// ======================== 模块级静态变量 ========================

static volatile uint32_t *s_heart = NULL;          /**< 心跳寄存器 */
static mpu6050_data_t *s_data = NULL;              /**< 六轴原始数据 */
static QueueHandle_t s_mpu6050_binary = NULL;      /**< 采集完成信号量 */
static three_angels *s_angels = NULL;              /**< 输出: 姿态角 */
static SemaphoreHandle_t s_angels_mutex = NULL;    /**< 保护姿态角的互斥锁 */
static QueueHandle_t s_servo_binary = NULL;        /**< 通知舵机任务的信号量 */

// ======================== 任务配置常量 ========================

const static uint16_t task_stack_size = 1024;
const static UBaseType_t task_priority = 4;

// ======================== 任务函数 ========================

/**
 * @brief 姿态解算任务 —— 互补滤波
 *
 * 信号量驱动: 等待采集任务 give → 解算 → 写入共享 angels → give 舵机信号量
 */
static int mpu6050_task_calculate_data(void *pvParamters)
{
    static float filter_roll  = 0.0f;   /**< 互补滤波后的 Roll  (rad) */
    static float filter_pitch = 0.0f;   /**< 互补滤波后的 Pitch (rad) */
    static float filter_yaw   = 0.0f;   /**< 陀螺仪积分 Yaw       (rad) */
    static TickType_t last_tick = 0;    /**< 上一次采样时刻 */
    const float alpha = 0.98f;          /**< 互补滤波系数 (陀螺仪权重) */

    while(1)
    {
        // 阻塞等待采集任务通知
        if(xSemaphoreTake(s_mpu6050_binary, portMAX_DELAY))
        {
            // ---- 计算时间间隔 dt ----
            TickType_t now = xTaskGetTickCount();
            if (last_tick == 0) { last_tick = now; continue; }
            float dt = (now - last_tick) * portTICK_PERIOD_MS / 1000.0f;
            last_tick = now;

            // ---- 原始数据转换为物理量 ----
            // 加速度: ±16g 量程 → 2048 LSB/g
            float ax = s_data->accel_x / 2048.0f;
            float ay = s_data->accel_y / 2048.0f;
            float az = s_data->accel_z / 2048.0f;
            // 陀螺仪: ±2000°/s 量程 → 16.4 LSB/(°/s), 转为 rad/s
            float gx = (s_data->gyro_x / 16.4f) * PI / 180.0f;
            float gy = (s_data->gyro_y / 16.4f) * PI / 180.0f;
            float gz = (s_data->gyro_z / 16.4f) * PI / 180.0f;

            // ---- 加速度计计算静态 Roll/Pitch ----
            float accel_roll, accel_pitch, tmp;
            arm_atan2_f32(ay, az, &accel_roll);              // Roll  = atan2(ay, az)
            arm_sqrt_f32(ay * ay + az * az, &tmp);
            arm_atan2_f32(-ax, tmp, &accel_pitch);           // Pitch = atan2(-ax, sqrt(ay²+az²))

            // ---- 互补滤波 ----
            filter_roll  = alpha * (filter_roll  + gx * dt) + (1.0f - alpha) * accel_roll;
            filter_pitch = alpha * (filter_pitch + gy * dt) + (1.0f - alpha) * accel_pitch;
            filter_yaw  += gz * dt;                          // Yaw: 纯陀螺仪积分

            // ---- 转为角度制 ----
            float roll_deg  = filter_roll  * 180.0f / PI;
            float pitch_deg = filter_pitch * 180.0f / PI;
            float yaw_deg   = filter_yaw   * 180.0f / PI;

            // ---- 临界区: 写入共享姿态数据 ----
            xSemaphoreTake(s_angels_mutex, portMAX_DELAY);
            s_angels->roll = roll_deg;
            s_angels->pitch = pitch_deg;
            s_angels->yaw = yaw_deg;
            xSemaphoreGive(s_angels_mutex);

            // ---- 通知下游任务 ----
            *s_heart |= MPU6050_TASK_CALCULATE_DATA_STATUS;
            xSemaphoreGive(s_servo_binary);   // 触发舵机任务
        }
    }
    return 0;
}

// ======================== 对外接口 ========================

/**
 * @brief 创建 MPU6050 姿态解算任务
 * @param heart            心跳寄存器引用
 * @param data             六轴原始数据引用
 * @param mpu6050_binary   采集完成信号量（NULL 则返回 -1）
 * @param angels           输出姿态角引用
 * @param angels_mutex     姿态角互斥锁（NULL 则返回 -1）
 * @param servo_binary     舵机触发信号量（NULL 则返回 -1）
 * @return 0=成功, -1=参数无效
 */
int create_task_mpu6050_calculate(volatile uint32_t &heart,
                                  mpu6050_data_t &data,
                                  QueueHandle_t mpu6050_binary,
                                  three_angels &angels,
                                  SemaphoreHandle_t angels_mutex,
                                  QueueHandle_t servo_binary)
{
    if(!mpu6050_binary || !angels_mutex || !servo_binary) return -1;
    s_heart = &heart;
    s_data = &data;
    s_mpu6050_binary = mpu6050_binary;
    s_angels = &angels;
    s_angels_mutex = angels_mutex;
    s_servo_binary = servo_binary;

    return xTaskCreate(
        (TaskFunction_t)mpu6050_task_calculate_data,
        (const char *)"mpu6050_task_calculate_data",
        task_stack_size,
        (void *)NULL,
        task_priority,
        NULL
    );
}
