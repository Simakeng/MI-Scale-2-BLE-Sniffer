#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <status_led.h>
#include <udp_task.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <wifi.h>


#define TAG "main"

void app_main()
{
    ESP_LOGI(TAG, "Device bootup!");

    // Do system initialization
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Initialize LED task...");

    init_led_task();

    led_set_color(255, 255, 255);
    led_set_mode(LED_MODE_CONSTANT);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Initialize UDP task...");

    init_udp_task();

    ESP_LOGI(TAG, "Scan for WiFi APs...");

    

    led_set_color(0, 0, 255);
    led_set_mode(LED_MODE_BREATH);
}