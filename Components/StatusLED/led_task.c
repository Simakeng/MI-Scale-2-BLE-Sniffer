#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_check.h>
#include <esp_log.h>

#include "driver/ledc.h"
#include "status_led.h"

TaskHandle_t led_task_handle = NULL;

static const char *TAG = "led";

#define LED_R_PIN 3
#define LED_R_CHANNEL LEDC_CHANNEL_0
#define LED_G_PIN 4
#define LED_G_CHANNEL LEDC_CHANNEL_1
#define LED_B_PIN 5
#define LED_B_CHANNEL LEDC_CHANNEL_2

const uint32_t LED_PWM_RESOLUTION = LEDC_TIMER_12_BIT;
volatile struct
{
    uint8_t r, g, b;
    ledc_mode_t mode;
    uint32_t blink_period;
    uint32_t blink_duty;
    uint32_t breath_period;
} led_config;

// Gamma correction
// out = in^(2.2)
int32_t gamma_correction(int32_t value_q15)
{
    int32_t t_q15 = value_q15;
    int32_t t2_q15 = (t_q15 * t_q15) >> 15;
    int32_t t3_q15 = (t2_q15 * t_q15) >> 15;
    int32_t t4_q15 = (t2_q15 * t2_q15) >> 15;

    const int32_t a0 = -0.0177348 * (1 << 15);
    const int32_t a1 = 0.744491 * (1 << 15);
    const int32_t a2 = 0.37206 * (1 << 15);
    const int32_t a3 = -0.0991667 * (1 << 15);

    int32_t out_q15 = a0 * t_q15 + a1 * t2_q15 + a2 * t3_q15 + a3 * t4_q15;
    out_q15 >>= 15;

    if (out_q15 > 0x00080000)
        out_q15 = 0x00080000;
    if (out_q15 < 0)
        out_q15 = 0;
    return out_q15;
}

void led_update_value(uint8_t r, uint8_t g, uint8_t b)
{
    static const int32_t full = 1 << LED_PWM_RESOLUTION;

    // This conversion is based on the documentation of the LEDC library
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html#change-pwm-duty-cycle-using-software
    // the range of the duty is not 0 ~ 2**duty_resolution-1,
    // but 0 ~ (2**duty_resolution), so the value of it should be scale to range.
    int32_t r_duty_q15 = (r << 15) / 255;
    int32_t r_duty_corrected = gamma_correction(r_duty_q15);
    int32_t r_duty = (r_duty_corrected * full) >> 15;

    int32_t g_duty_q15 = (g << 15) / 255;
    int32_t g_duty_corrected = gamma_correction(g_duty_q15);
    int32_t g_duty = (g_duty_corrected * full) >> 15;

    int32_t b_duty_q15 = (b << 15) / 255;
    int32_t b_duty_corrected = gamma_correction(b_duty_q15);
    int32_t b_duty = (b_duty_corrected * full) >> 15;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_R_CHANNEL, full - r_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_R_CHANNEL);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_G_CHANNEL, full - g_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_G_CHANNEL);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_B_CHANNEL, full - b_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_B_CHANNEL);
}

void led_set_off(void)
{
    led_update_value(0, 0, 0);
}

void led_update_blink()
{
    int32_t tickCount = xTaskGetTickCount();
    tickCount %= 1000; // 1s period

    if (tickCount > 100)
        led_set_off();
    else
    {
        led_update_value(led_config.r, led_config.g, led_config.b);
    }
}

void led_update_breath()
{
    int32_t tickCount = xTaskGetTickCount();
    const int32_t period = 500; // 5s period
    tickCount %= period;

    // make it a triangle wave
    //  1     #     #
    //       # #   # #
    //      #   # #   #
    //  0  #     #     #
    //    0   period  2*period
    if (tickCount > (period / 2))
        tickCount = period - tickCount;
    int32_t t_q15 = (tickCount << 15) / (period / 2);

    // (3 - 2 t) t^2 lerping(approximate to 1/2 - 1/2cos[pi*t])
    int32_t t_square_q15 = (t_q15 * t_q15) >> 15;
    t_q15 = ((3 * 0x00008000 - 2 * t_q15) * t_square_q15) >> 15;
    uint32_t r, g, b;

    r = (led_config.r * t_q15) >> 15;
    g = (led_config.g * t_q15) >> 15;
    b = (led_config.b * t_q15) >> 15;

    // update value
    led_update_value(r, g, b);
}

void led_update_constant()
{
    led_update_value(led_config.r, led_config.g, led_config.b);
}

void led_task_main()
{
    // start of the task
    ESP_LOGI(TAG, "LED task started!\n");

    // init LED Controller timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LED_PWM_RESOLUTION,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // init LED Controller channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = 0,
        .duty = 255,
        .hpoint = 0,
    };

    ledc_channel.channel = LED_R_CHANNEL;
    ledc_channel.gpio_num = LED_R_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_channel.channel = LED_G_CHANNEL;
    ledc_channel.gpio_num = LED_G_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_channel.channel = LED_B_CHANNEL;
    ledc_channel.gpio_num = LED_B_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI(TAG, "LED started!\n");

    while (1)
    {
        // update the LED
        vTaskDelay(1); // 10ms delay

        led_mode_t mode = led_config.mode;
        switch (mode)
        {
        case LED_MODE_OFF:
            led_set_off();
            break;
        case LED_MODE_BLINK:
            led_update_blink();
            break;
        case LED_MODE_BREATH:
            led_update_breath();
            break;
        case LED_MODE_CONSTANT:
            led_update_constant();
            break;
        }
    }
}

esp_err_t init_led_task(void)
{
    // Prevent multiple initializations
    if (led_task_handle != NULL)
        return ESP_OK;

    memset((void *)&led_config, 0, sizeof(led_config));

    xTaskCreate(led_task_main, "led_task", 4096, NULL, 5, &led_task_handle);

    if (led_task_handle == NULL)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t led_set_mode(led_mode_t mode)
{
    led_config.mode = mode;
    return ESP_OK;
}

esp_err_t led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    led_config.r = red;
    led_config.g = green;
    led_config.b = blue;
    return ESP_OK;
}
