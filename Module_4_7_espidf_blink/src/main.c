#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

// Define the GPIO pin for the LED (GPIO 2 is common for onboard LEDs)
#define BLINK_GPIO 18
static const char *TAG = "ESP_LOG";

volatile uint16_t global_count = 0;

void print_count(uint16_t count)
{
	printf("count= %u\n", count);
}

void app_main(void)
{
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << BLINK_GPIO),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};

	gpio_config(&io_conf);
	volatile uint16_t local_count = 0;

	// Blink loop
	while (1)
	{

		global_count++;
		local_count++;

		// Turn LED ON
		printf("LED ON\n");
		ESP_LOGI(TAG, "LED ON\n");
		gpio_set_level(BLINK_GPIO, 1);
		vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 second

		// Turn LED OFF
		printf("LED OFF\n");
		ESP_LOGI(TAG, "LED OFF\n");
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 second
		print_count(local_count);
	}
}
