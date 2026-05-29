#include <string>
#include <cstdint>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "usart.h"

using namespace etl;

//发送缓冲区
vector<char, 1024>cmt_tx_buf;
//接收缓冲区
vector<char, 1024>cmt_rx_buf;
