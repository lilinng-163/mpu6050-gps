/**
 * @file    oled_208.hpp
 * @brief   OLED SH1122 驱动声明 (256×64, 4-bit 灰度, I2C)
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_i2c.hpp"

using std::string;

/**
 * @brief OLED SH1122 驱动类 (256×64, 4-bit 灰度)
 *
 * 每字节存 2 像素: 高 nibble=偶数列, 低 nibble=奇数列
 */
class oled_208 : public my_i2c
{
public:
    oled_208(GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int set_pixel(uint16_t x, uint16_t y);     /**< 点亮像素 */
    int clear_pixel(uint16_t x, uint16_t y);   /**< 熄灭像素 */
    int clear(void);                            /**< 清屏 */
    int refresh(void);                          /**< 刷新显示 */
    int show_string(std::string str, uint16_t x, uint16_t y);  /**< 显示字符串 */
    int show_num(int num, uint16_t x, uint16_t y);             /**< 显示整数 */
    ~oled_208(void);

private:
    static constexpr uint16_t WIDTH  = 256;       /**< 屏幕宽度 */
    static constexpr uint16_t HEIGHT = 64;        /**< 屏幕高度 */

    // SH1122: 4-bit 灰度, 每字节存 2 像素
    // 每行 128 字节, 共 64 行 → 8192 bytes
    static constexpr uint16_t BYTE_PER_ROW = WIDTH / 2;  /**< 每行字节数 */
    static constexpr uint16_t ROW_CNT      = HEIGHT;     /**< 行数 */

    unsigned char buffer[ROW_CNT * BYTE_PER_ROW];   /**< 显存 8192 bytes */
    GPIO_TypeDef *gpiox;            /**< GPIO 端口 */
    uint16_t scl;                   /**< SCL 引脚 */
    uint16_t sda;                   /**< SDA 引脚 */
    int write_cmd(unsigned char cmd);                    /**< 写命令 */
    int write_data(unsigned char data);                  /**< 写单字节数据 */
    int write_data_bulk(unsigned char *data, int len);   /**< 批量写数据 */
    int setcursor(unsigned char x, unsigned char y);     /**< 设置光标 */
};