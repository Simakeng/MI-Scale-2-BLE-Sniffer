/**
 * @file wifi.h
 * @author simakeng (simakeng@outlook.com)
 * @brief Wifi connection and management
 * @version 0.1
 * @date 2024-05-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <esp_err.h>
#include <stdbool.h>

esp_err_t wifi_init_start(void);

esp_err_t wifi_connect(void);

esp_err_t wifi_wait_connected(uint32_t timeout_ms);

bool wifi_is_connected(void);