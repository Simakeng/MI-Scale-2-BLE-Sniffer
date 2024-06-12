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

#include <lwip/sockets.h>

#include "wifi.h"

#define TAG "udp"

#define DST_ADDR_BRDCST "255.255.255.255"

esp_err_t init_udp_task()
{
    ESP_LOGV(TAG, "Starting WiFi...");

    ESP_ERROR_CHECK(wifi_init_start());

    ESP_LOGV(TAG, "Connecting to WiFi...");

    ESP_ERROR_CHECK(wifi_connect());

    ESP_LOGV(TAG, "Waiting for WiFi connection...");

    ESP_ERROR_CHECK(wifi_wait_connected(5 * 60000));

    ESP_LOGI(TAG, "WiFi connected!");

    ESP_LOGI(TAG, "Start to send data...");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    // broadcast
    server_addr.sin_addr.s_addr = inet_addr(DST_ADDR_BRDCST);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create socket: %d", sock);
        return ESP_FAIL;
    }

    while (true)
    {
        const char *msg = "Hello, World!";
        int err = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (err < 0)
        {
            ESP_LOGE(TAG, "Failed to send message: %d", err);
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "%d bytes send.", err);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    return ESP_OK;
}