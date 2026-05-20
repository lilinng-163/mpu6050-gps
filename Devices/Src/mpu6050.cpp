#include <cstdio>
#include <cstdint>
#include <etl/vector.h>
#include "stm32f1xx_hal.h"
#include "mpu6050.hpp"

mpu6050::mpu6050(GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda)
    : my_i2c("mpu6050_i2c", _gpiox, _scl, _sda)
{
    addr_r = addr_w | 0x01;
    init();
}

int mpu6050::init(void)
{
    unsigned char val;

    // 1. 唤醒设备，选择内部时钟源
    val = 0x01;
    if(send_data(addr_w, PWR_MGMT_1, &val, 1) != 0)
        return -1;

    // 2. 使能所有轴
    val = 0x00;
    if(send_data(addr_w, PWR_MGMT_2, &val, 1) != 0)
        return -2;

    // 3. 配置采样率：1kHz (8kHz / (9+1) = 800Hz)
    val = 0x09;
    if(send_data(addr_w, SMPLRT_DIV, &val, 1) != 0)
        return -3;

    // 4. 配置数字低通滤波器(DLPF)
    val = 0x06;
    if(send_data(addr_w, CONFIG, &val, 1) != 0)
        return -4;

    // 5. 配置陀螺仪量程：±2000 °/s
    val = 0x18;
    if(send_data(addr_w, GYRO_CONFIG, &val, 1) != 0)
        return -5;

    // 6. 配置加速度计量程：±16g
    val = 0x18;
    if(send_data(addr_w, ACCEL_CONFIG, &val, 1) != 0)
        return -6;

    return 0;
}

int mpu6050::set_device_id(unsigned char _addr_w, unsigned char _addr_r)
{
    addr_w = _addr_w;
    addr_r = _addr_r;
    return 0;
}

int mpu6050::get_data(mpu6050_data_t &data)
{
    unsigned char buf[14];
    if(receive_data(addr_w, ACCEL_XOUT_H, buf, 14) != 0)
        return -1;

    data.accel_x = (buf[0]  << 8) | buf[1];
    data.accel_y = (buf[2]  << 8) | buf[3];
    data.accel_z = (buf[4]  << 8) | buf[5];
    data.temp    = (buf[6]  << 8) | buf[7];
    data.gyro_x  = (buf[8]  << 8) | buf[9];
    data.gyro_y  = (buf[10] << 8) | buf[11];
    data.gyro_z  = (buf[12] << 8) | buf[13];

    return 0;
}

