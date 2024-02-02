#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_check.h>
#include <esp_log.h>

TaskHandle_t led_task_handle;

void led_task_main()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI("led", "LED task started!\n");
    }
}

esp_err_t init_led_task(void)
{
    led_task_handle = NULL;
    xTaskCreate(led_task_main, "led_task", 4096, NULL, 5, &led_task_handle);

    if (led_task_handle == NULL)
        return ESP_FAIL;

    return ESP_OK;
}
