#pragma once

#include <string>
#include <cstdint>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include "stm32f1xx_hal.h"
#include "usart.h"

using std::string;

class gps_data_t
{
public:
    bool valid;
    double latitude;
    double longitude;
    float speed;
    float course;
    float altitude;
    unsigned char satellites;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
};

class gps_module
{
public:
    gps_module(void);
    etl::circular_buffer<char, 96>rx_buf;
    int dump_raw(string &buffer);
    bool feed(unsigned char byte);
    int get_data(gps_data_t &gps_data);
private:
    gps_data_t gps_data;
};