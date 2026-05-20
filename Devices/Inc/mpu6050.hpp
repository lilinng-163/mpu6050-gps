#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_i2c.hpp"

using std::string;

/*定义寄存器地址
   define ADDRESS of necessary register 
*/
#define SMPLRT_DIV 		0x19
#define CONFIG 	   		0x1A
#define GYRO_CONFIG 	0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_XOUT_H 	0x3B
#define ACCEL_XOUT_L 	0x3C
#define ACCEL_YOUT_H 	0x3D
#define ACCEL_YOUT_L	0x3E
#define ACCEL_ZOUT_H 	0x3F
#define ACCEL_ZOUT_L	0x40
#define TEMP_OUT_H 		0x41
#define TEMP_OUT_L		0x42
#define GYRO_XOUT_H		0x43
#define GYRO_XOUT_L		0x44
#define GYRO_YOUT_H		0x45
#define GYRO_YOUT_L		0x46
#define GYRO_ZOUT_H		0x47
#define GYRO_ZOUT_L		0x48
/*定义电源配置寄存器地址
  define ADDRESS of POWER managerment register
*/
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C

/*加速度计/陀螺仪量程配置
  Accelerometer and Gyroscope range config
*/
#define ACCEL_RANGE_2G  0x00
#define ACCEL_RANGE_4G  0x08
#define ACCEL_RANGE_8G  0x10
#define ACCEL_RANGE_16G 0x18

#define GYRO_RANGE_250  0x00
#define GYRO_RANGE_500  0x08
#define GYRO_RANGE_1000 0x10
#define GYRO_RANGE_2000 0x18


typedef struct
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
}mpu6050_data_t;

class mpu6050 : public my_i2c
{
public:
    mpu6050(GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int init(void);    //初始化MPU6050(唤醒+配置)
    int set_device_id(unsigned char _addr_w, unsigned char _addr_r);    //更改设备id(w/r)
    int get_data(mpu6050_data_t &data);    //取出六轴信息
private:
    unsigned char addr_w = 0xD0;    //默认设备地址(写)是0xD0
    unsigned char addr_r;
};