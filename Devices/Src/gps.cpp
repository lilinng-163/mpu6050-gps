#include <string>
#include <cstdint>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "gps.hpp"

using namespace etl;
using std::string;

gps_module::gps_module(void)
{
    rx_buf.clear();
}
int gps_module::dump_raw(string &buffer)
{
    taskENTER_CRITICAL();
    if(!buffer.empty())
    {
        buffer.clear();
    }

    while (rx_buf.size() > 0)
    {
        buffer += rx_buf.front();
        rx_buf.pop();
    }

    taskEXIT_CRITICAL();
    return 0;
}
bool gps_module::feed(unsigned char byte)
{
    return true;
}
int gps_module::get_data(gps_data_t &gps_data)
{
    return 0;
}
