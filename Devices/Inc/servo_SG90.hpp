#pragma once

#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "my_pwm.hpp"

using std::string;

class servo_sg90 : public my_pwm
{
public:
    servo_sg90(TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty);
    int set_duty(float duty);
private:
    uint16_t period = 20;   //ms
    float max_duty = 12.5;    
}