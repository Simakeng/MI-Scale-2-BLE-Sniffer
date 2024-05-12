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

/****************************************
 * HARD CODED CONFIGURATION
 */

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASS"
#define WIFI_DEVICE_DESC "ESP32-UDP-Client"

// remove this or make your own when you see this file is not found
// it is used to override the WIFI_SSID and WIFI_PASS above
#include ".private-configs.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_wifi.h>
#define TAG "udp"

static esp_netif_t *netif = NULL;

esp_err_t init_wifi_start(void)
{
    esp_err_t ret = ESP_OK;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    ESP_ERROR_CHECK(ret);

    esp_netif_inherent_config_t netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    netif_config.if_desc = WIFI_DEVICE_DESC;
    netif_config.route_prio = 128;

    netif = esp_netif_create_wifi(WIFI_IF_STA, &netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ret;
}

esp_err_t init_udp_task()
{
    ESP_LOGI(TAG, "Starting WiFi...");
    ESP_ERROR_CHECK(init_wifi_start());


    return ESP_OK;
}