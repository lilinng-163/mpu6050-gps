/**
 * @file    oled_096.hpp
 * @brief   OLED SSD1306 驱动声明 (128×64, I2C)
 * @date    2026-06-06
 */

#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_i2c.hpp"

using std::string;

/**
 * @brief OLED SSD1306 驱动类 (128×64 单色)
 */
class oled : public my_i2c
{
public:
    oled(GPIO_TypeDef *_gpiox, uint16_t _scl, uint16_t _sda);
    int set_pixel(uint16_t x, uint16_t y);     /**< 点亮像素 */
    int clear_pixel(uint16_t x, uint16_t y);   /**< 熄灭像素 */
    int clear(void);                            /**< 清屏 */
    int refresh(void);                          /**< 刷新显示 */
    int show_string(std::string str, uint16_t x, uint16_t y);  /**< 显示字符串 (8×16) */
    int show_num(int num, uint16_t x, uint16_t y);             /**< 显示整数 */
    ~oled(void);
private:
    unsigned char buffer[1024];     /**< 显存 128×64 = 1024 bytes */
    GPIO_TypeDef *gpiox;            /**< GPIO 端口 */
    uint16_t scl;                   /**< SCL 引脚 */
    uint16_t sda;                   /**< SDA 引脚 */
    int write_cmd(unsigned char cmd);    /**< 写命令 */
    int write_data(unsigned char data);  /**< 写数据 */
    int setcursor(unsigned char x, unsigned char y);  /**< 设置光标 */
};