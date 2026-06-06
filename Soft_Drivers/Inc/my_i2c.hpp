/**
 * @file    my_i2c.hpp
 * @brief   软件 I2C 驱动声明 —— GPIO 模拟 I2C 主设备
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"

using std::string;

/**
 * @brief 软件 I2C 主设备驱动类
 *
 * 通过 GPIO 模拟 I2C 时序，支持标准写/读事务。
 */
class my_i2c
{
public:
    my_i2c(string _self_name, GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int enable_delay(void);       /**< 启用 ACK 读取时的延时 */
    string get_name(void);        /**< 获取实例名称 */

    /**
     * @brief I2C 写事务: START + 设备地址(W) + 寄存器地址 + 数据... + STOP
     * @return 0=成功, -1=ACK 错误
     */
    int send_data(unsigned char dev_addr_w, unsigned char reg_addr,
                  unsigned char *send_data, int len);

    /**
     * @brief I2C 读事务: START + 设备地址(W) + 寄存器地址 + RESTART + 设备地址(R) + 数据... + NACK + STOP
     * @return 0=成功, -1=ACK 错误
     */
    int receive_data(unsigned char dev_addr_w, unsigned char reg_addr,
                     unsigned char *receive_data, int len);

private:
    string self_name;               /**< 实例名称（调试用） */
    GPIO_TypeDef *gpiox;            /**< GPIO 端口 */
    uint16_t scl;                   /**< SCL 引脚 */
    uint16_t sda;                   /**< SDA 引脚 */
    int is_delay = 0;               /**< 是否在 ACK 读取时插入延时 */

    // 底层时序
    int start(void);                              /**< 发送 START 条件 */
    int stop(void);                               /**< 发送 STOP 条件 */
    int send_ack(unsigned char ack);              /**< 发送 ACK/NACK */
    unsigned char receive_ack(void);              /**< 接收 ACK */
    int send_byte(unsigned char byte);            /**< 发送一个字节 */
    unsigned char receive_byte(void);             /**< 接收一个字节 */
};