/**
 * @file    mpu6050.hpp
 * @brief   MPU6050 六轴传感器驱动声明 —— 寄存器定义、数据结构、驱动类
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_i2c.hpp"

using std::string;

// ======================== 寄存器地址 ========================

#define SMPLRT_DIV 		0x19    /**< 采样率分频器 */
#define CONFIG 	   		0x1A    /**< 数字低通滤波器配置 */
#define GYRO_CONFIG 	0x1B    /**< 陀螺仪量程配置 */
#define ACCEL_CONFIG    0x1C    /**< 加速度计量程配置 */
#define ACCEL_XOUT_H 	0x3B    /**< 加速度 X 轴高字节 */
#define ACCEL_XOUT_L 	0x3C    /**< 加速度 X 轴低字节 */
#define ACCEL_YOUT_H 	0x3D    /**< 加速度 Y 轴高字节 */
#define ACCEL_YOUT_L	0x3E    /**< 加速度 Y 轴低字节 */
#define ACCEL_ZOUT_H 	0x3F    /**< 加速度 Z 轴高字节 */
#define ACCEL_ZOUT_L	0x40    /**< 加速度 Z 轴低字节 */
#define TEMP_OUT_H 		0x41    /**< 温度高字节 */
#define TEMP_OUT_L		0x42    /**< 温度低字节 */
#define GYRO_XOUT_H		0x43    /**< 陀螺仪 X 轴高字节 */
#define GYRO_XOUT_L		0x44    /**< 陀螺仪 X 轴低字节 */
#define GYRO_YOUT_H		0x45    /**< 陀螺仪 Y 轴高字节 */
#define GYRO_YOUT_L		0x46    /**< 陀螺仪 Y 轴低字节 */
#define GYRO_ZOUT_H		0x47    /**< 陀螺仪 Z 轴高字节 */
#define GYRO_ZOUT_L		0x48    /**< 陀螺仪 Z 轴低字节 */

// ======================== 电源管理寄存器 ========================

#define PWR_MGMT_1 0x6B          /**< 电源管理 1 (唤醒/时钟源) */
#define PWR_MGMT_2 0x6C          /**< 电源管理 2 (轴使能) */

// ======================== 量程配置值 ========================

#define ACCEL_RANGE_2G  0x00     /**< 加速度 ±2g */
#define ACCEL_RANGE_4G  0x08     /**< 加速度 ±4g */
#define ACCEL_RANGE_8G  0x10     /**< 加速度 ±8g */
#define ACCEL_RANGE_16G 0x18     /**< 加速度 ±16g */

#define GYRO_RANGE_250  0x00     /**< 陀螺仪 ±250°/s */
#define GYRO_RANGE_500  0x08     /**< 陀螺仪 ±500°/s */
#define GYRO_RANGE_1000 0x10     /**< 陀螺仪 ±1000°/s */
#define GYRO_RANGE_2000 0x18     /**< 陀螺仪 ±2000°/s */

// ======================== 数据结构 ========================

/**
 * @brief 姿态角 (欧拉角, 角度制)
 */
typedef struct
{
  float roll;     /**< 横滚角 (度) */
  float pitch;    /**< 俯仰角 (度) */
  float yaw;      /**< 偏航角 (度) */
}three_angels;

/**
 * @brief MPU6050 原始六轴数据 (ADC 原始值)
 */
typedef struct
{
    int16_t accel_x;   /**< 加速度 X 轴 */
    int16_t accel_y;   /**< 加速度 Y 轴 */
    int16_t accel_z;   /**< 加速度 Z 轴 */
    int16_t temp;      /**< 温度 */
    int16_t gyro_x;    /**< 陀螺仪 X 轴 */
    int16_t gyro_y;    /**< 陀螺仪 Y 轴 */
    int16_t gyro_z;    /**< 陀螺仪 Z 轴 */
}mpu6050_data_t;

// ======================== 驱动类 ========================

/**
 * @brief MPU6050 驱动类（软件 I2C）
 */
class mpu6050 : public my_i2c
{
public:
    mpu6050(GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int set_device_id(unsigned char _addr_w, unsigned char _addr_r);    /**< 更改设备 I2C 地址 */
    int get_data(mpu6050_data_t &data);                                  /**< 读取六轴原始数据 */
private:
    int init(void);                   /**< 初始化 MPU6050 (唤醒 + 配置寄存器) */
    unsigned char addr_w = 0xD0;      /**< 设备地址(写), 默认 0xD0 */
    unsigned char addr_r;             /**< 设备地址(读) = addr_w | 0x01 */
};

/**
 * @brief 姿态解算（已迁移到 task_mpu6050_calculate.cpp 的互补滤波实现）
 */
int calculate_data(mpu6050_data_t &mpu6050_data);