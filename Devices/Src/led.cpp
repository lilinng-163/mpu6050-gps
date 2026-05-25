#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "led.hpp"

led::led(GPIO_TypeDef *_gpiox, uint16_t _pin)
: gpiox(_gpiox), pin(_pin)
{
    is_on = !(bool)HAL_GPIO_ReadPin(gpiox, pin);
}
int led::on(void)
{
    HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_RESET);
    is_on = true;
    return 0;
}
int led::off(void)
{
    HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_SET);
    is_on = false;
    return 0;
}
bool led::get_status(void)
{
    return is_on;
}
