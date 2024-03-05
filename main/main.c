#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <status_led.h>

void app_main()
{
    ESP_LOGI("main", "Hello, world!");

    init_led_task();

    led_set_color(0, 0, 255);
    led_set_mode(LED_MODE_CONSTANT);
}