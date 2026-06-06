/**
 * @file    gps.hpp
 * @brief   GPS 模块驱动声明 —— 环形缓冲区 + 原始数据 dump + 解析接口
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "usart.h"

using std::string;

/**
 * @brief GPS 解析结果结构体
 */
class gps_data_t
{
public:
    bool valid;              /**< 数据是否有效 */
    double latitude;         /**< 纬度 (度) */
    double longitude;        /**< 经度 (度) */
    float speed;             /**< 速度 (节) */
    float course;            /**< 航向 (度) */
    float altitude;          /**< 海拔 (米) */
    unsigned char satellites;/**< 卫星数 */
    unsigned char hour;      /**< UTC 时 */
    unsigned char min;       /**< UTC 分 */
    unsigned char sec;       /**< UTC 秒 */
};

/**
 * @brief GPS 模块驱动类
 *
 * 数据流: USART3 IRQ → rx_buf (环形缓冲) → dump_raw() → 上层任务
 */
class gps_module
{
public:
    gps_module(void);
    etl::circular_buffer<char, 96>rx_buf;   /**< 96 字节环形接收缓冲区 */
    int dump_raw(string &buffer);            /**< 取出所有原始字节 */
    bool feed(unsigned char byte);           /**< 逐字节喂入 (预留) */
    int get_data(gps_data_t &gps_data);      /**< 解析 GPS 数据 (待实现) */
private:
    gps_data_t gps_data;
};