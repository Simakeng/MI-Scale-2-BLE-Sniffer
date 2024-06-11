/**
 * @file wifi.c
 * @author simakeng (simakeng@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <esp_wifi.h>
#include <esp_check.h>
#include <esp_netif.h>
#include <esp_event.h>

#define TAG "wifi"

/****************************************
 * HARD CODED CONFIGURATION
 */

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASS"
#define WIFI_DEVICE_DESC "ESP32-UDP-Client"

// remove this or make your own when you see this file is not found
// it is used to override the WIFI_SSID and WIFI_PASS above
#include ".private-configs.h"

static esp_netif_t *netif = NULL;

static struct
{
    volatile bool connected;
    esp_ip4_addr_t ip_addr;
    char ipv4_addr_str[20];
    SemaphoreHandle_t lock;
} wifi_status;

/**
 * @brief Event handler for wifi event IP_EVENT_STA_GOT_IP.
 * @see esp_event_handler_t
 * @param arg
 * @param event_base
 * @param event_id
 * @param event_data
 */
static void wifi_event_handler_got_ip(void *arg, esp_event_base_t event_base,
                                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

    // copy ip address
    wifi_status.ip_addr = event->ip_info.ip;
    sprintf(wifi_status.ipv4_addr_str, IPSTR, IP2STR(&event->ip_info.ip));

    // cheers
    ESP_LOGI(TAG, "Got IP Address: %s", wifi_status.ipv4_addr_str);

    // set connected flag
    wifi_status.connected = true;

    // release semaphore
    if (wifi_status.lock != NULL)
        xSemaphoreGive(wifi_status.lock);
}

/**
 * @brief Event handler for wifi event WIFI_EVENT_STA_CONNECTED.
 * 
 * @param esp_netif 
 * @param event_base 
 * @param event_id 
 * @param event_data 
 */
static void wifi_event_handler_connect(void *esp_netif, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data)
{
    ESP_LOGV(TAG, "Connected to AP");
}


/**
 * @brief Event handler for wifi event WIFI_EVENT_STA_DISCONNECTED.
 * 
 * @param esp_netif 
 * @param event_base 
 * @param event_id 
 * @param event_data 
 */
static void wifi_event_handler_disconnect(void *esp_netif, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data)
{
    wifi_status.connected = false;
    ESP_LOGV(TAG, "Disconnected from AP");
}

/**
 * @brief Initialize the wifi connection.
 * @return esp_err_t
 */
esp_err_t wifi_init_start(void)
{
    esp_err_t ret = ESP_OK;

    // init status
    memset(&wifi_status, 0, sizeof(wifi_status));

    // init wifi
    ESP_LOGD(TAG, "Init wifi config...");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    ESP_ERROR_CHECK(ret);

    // init netif
    ESP_LOGD(TAG, "Init netif...");
    esp_netif_inherent_config_t netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    netif_config.if_desc = "esp-netif-wifi-sta";
    netif_config.route_prio = 128;
    netif = esp_netif_create_wifi(WIFI_IF_STA, &netif_config);

    // change hostname
    ESP_ERROR_CHECK(esp_netif_set_hostname(netif, WIFI_DEVICE_DESC));

    // set handlers
    esp_wifi_set_default_wifi_sta_handlers();

    // create semaphore for waiting got ip
    wifi_status.lock = xSemaphoreCreateBinary();
    if (wifi_status.lock == NULL)
    {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return ESP_FAIL;
    }

    // start wifi
    ESP_LOGI(TAG, "Start wifi...");
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // resigter wifi event handler
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler_disconnect, NULL);
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_event_handler_connect, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler_got_ip, NULL);

    return ret;
}

/**
 * @brief Connect to the wifi predefined in the macro.
 *
 * @return esp_err_t
 */
esp_err_t wifi_connect(void)
{
    // select which wifi to connect to
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,

            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH,
        },
    };

    // start connection
    ESP_LOGI(TAG, "Connecting to %s...", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to connect to %s, ret:%x", WIFI_SSID, ret);
        return ret;
    }
    return ret;
}

/**
 * @brief Wait for given time for the wifi to connect.
 *
 * @param timeout_ms
 * @return esp_err_t
 */
esp_err_t wifi_wait_connected(uint32_t timeout_ms)
{
    if (xSemaphoreTake(wifi_status.lock, pdMS_TO_TICKS(timeout_ms)) != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to connect to %s", WIFI_SSID);
        return ESP_FAIL;
    }
    wifi_status.connected = true;
    return ESP_OK;
}


/**
 * @brief Check if wifi is connected.
 * 
 * @return true 
 * @return false 
 */
bool wifi_is_connected(void)
{
    return wifi_status.connected;
}