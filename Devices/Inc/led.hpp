#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"

class led
{
public:
    led(GPIO_TypeDef *_gpiox, uint16_t _pin);
    int on(void);
    int off(void);
    bool get_status(void);
private:
    bool is_on;
    GPIO_TypeDef *gpiox;
    uint16_t pin;
};