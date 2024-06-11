/**
 * @file udp_task.c
 * @author simakeng (simakeng@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <esp_check.h>
#include <esp_log.h>

#include "wifi.h"

#define TAG "udp"

esp_err_t init_udp_task()
{
    ESP_LOGV(TAG, "Starting WiFi...");

    ESP_ERROR_CHECK(wifi_init_start());

    ESP_LOGV(TAG, "Connecting to WiFi...");

    ESP_ERROR_CHECK(wifi_connect());

    ESP_LOGV(TAG, "Waiting for WiFi connection...");

    ESP_ERROR_CHECK(wifi_wait_connected(5*60000));

    return ESP_OK;
}