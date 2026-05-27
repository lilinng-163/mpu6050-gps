#pragma once

#include <cstdint>
#include <string>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif
void vConfigureTimerForRunTimeStats(void);
uint32_t ulGetRunTimeCounterValue(void);

#ifdef __cplusplus
}
#endif