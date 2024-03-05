#include <esp_check.h>
#include <esp_log.h>

esp_err_t init_led_task(void);

esp_err_t led_set_color(uint8_t red, uint8_t green, uint8_t blue);

typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_BLINK,
    LED_MODE_BREATH,
    LED_MODE_CONSTANT,
} led_mode_t;

esp_err_t led_set_mode(led_mode_t mode);
