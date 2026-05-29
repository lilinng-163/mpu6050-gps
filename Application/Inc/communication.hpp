#pragma once

#include <etl/vector.h>

using namespace etl;

/*用于uart2和esp32的通信*/
extern vector<char, 1024>cmt_tx_buf;
extern vector<char, 1024>cmt_rx_buf;