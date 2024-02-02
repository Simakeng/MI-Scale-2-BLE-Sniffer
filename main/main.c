#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <status_led.h>

void app_main()
{
    ESP_LOGI("main", "Hello, world!");

    init_led_task();
}