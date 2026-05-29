#pragma once

#include <cstdint>


#define CPU_STATS_TASK_STATUS                       (1 << 0)
#define LED_TASK_BLINK_STATUS                       (1 << 1)
#define SERVO_TASK_CONTROL_STATUS                   (1 << 2)
#define MPU6050_TASK_CALCULATE_DATA_STATUS          (1 << 3)
#define MPU6050_TASK_COLLECT_DATA_STATUS            (1 << 4)
#define OLED_TASK_STATUS                            (1 << 5)
#define GPS_TASK_STATUS                             (1 << 6)
#define CMT_TASK_STATUS                             (1 << 7)



