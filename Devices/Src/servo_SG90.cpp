#include <string>
#include <cstdint>
#include "stm32f1xx_hal.h"
#include "servo_SG90.hpp"

servo_sg90::servo_sg90(TIM_HandleTypeDef *_htim, uint32_t _channel, float _duty)
: my_pwm("servo_sg90_pwm", _htim, _channel, _duty)
{

}
int servo_sg90::set_duty(float duty)
{
    // if(duty < 0.0f || duty > 12.5f)
    // {
    //     return -1;
    // }

    auto res = my_pwm::set_duty(duty);
    
    if(res != 0)
    {
        return res;
    }

    return 0;
}