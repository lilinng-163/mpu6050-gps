#include <cstdint>
#include "tim6_get.hpp"


void vConfigureTimerForRunTimeStats(void)
{
    
}

uint32_t ulGetRunTimeCounterValue(void)
{
    static uint16_t last = 0;
    static uint32_t high = 0;
    uint16_t now = TIM6->CNT;

    if(now < last)
    { 
        high += 0x10000;
    }
    last = now;

    return high + now;
}