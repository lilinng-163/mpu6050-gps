#include <string>
#include <cstdint>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "gps.hpp"

using namespace etl;
using std::string;

uint8_t gps_rx_byte = 0x00;
gps_module gm;

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        //先读取新的字节再重新接收
        gm.rx_buf.push((char)gps_rx_byte);
        HAL_UART_Receive_IT(&huart3, &gps_rx_byte, 1);
    }
}