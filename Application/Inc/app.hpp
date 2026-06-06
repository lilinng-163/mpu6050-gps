/**
 * @file    app.hpp
 * @brief   应用层入口声明
 * @date    2026-06-06
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 启动 FreeRTOS —— 由 main.c 调用，创建所有同步原语和任务并启动调度器
 * @return 0=成功, 非0=失败
 */
int start_freertos(void);

#ifdef __cplusplus
}
#endif
