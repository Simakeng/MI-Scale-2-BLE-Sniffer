#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <status_led.h>

void app_main()
{
    ESP_LOGI("main", "Device bootup!");

    init_led_task();

    led_set_color(255, 255, 255);
    led_set_mode(LED_MODE_CONSTANT);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI("main", "Scan for WiFi APs...");

    led_set_color(0, 0, 255);
    led_set_mode(LED_MODE_BREATH);
}