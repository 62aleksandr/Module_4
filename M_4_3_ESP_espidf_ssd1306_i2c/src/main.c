// https://components.espressif.com/components/esp-idf-lib/ds1307/versions/1.0.7/readme?language=en
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <i2cdev.h>
#include "ssd1306.h"
#include <string.h>

// Конфігурація I2C
#define I2C_PORT I2C_NUM_0
#define I2C_MASTER_SDA_IO 16
#define I2C_MASTER_SCL_IO 15

static const char *TAG = "OLED_MAIN";

void app_main(void)
{
	// 1. Створення та налаштування дескриптора дисплея
	ssd1306_t oled_dev;
	memset(&oled_dev, 0, sizeof(ssd1306_t));

	// 2. Ініціалізація системного драйвера i2cdev
	ESP_ERROR_CHECK(i2cdev_init());

	esp_err_t err = ssd1306_init_desc(&oled_dev, I2C_PORT,
									  I2C_MASTER_SDA_IO,
									  I2C_MASTER_SCL_IO);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init descriptor: %d", err);
		return;
	}

	// 3. Апаратна ініціалізація чипа SSD1306
	err = ssd1306_init_display(&oled_dev);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init display: %d", err);
		return;
	}

	vTaskDelay(pdMS_TO_TICKS(100));

	// 4. Очищення екрана перед виводом інформації
	ssd1306_clear(&oled_dev);

	// Вивід статичного заголовка (на 0-ю сторінку екрана)
	ssd1306_draw_string(&oled_dev, 0, 0, "ESP32-S3 OLED");
	ssd1306_draw_string(&oled_dev, 0, 1, "SYSTEM READY");

	uint32_t counter = 0;
	char text_buf[32];

	while (1)
	{
		// Форматируем цифри рахунку в текстову рядок
		snprintf(text_buf, sizeof(text_buf), "COUNTER: %lu", counter);

		// Виводим оновлювані цифри на 4-ю сторінку (рядок) дисплея
		ssd1306_draw_string(&oled_dev, 0, 4, text_buf);

		ESP_LOGI(TAG, "Display updated: %s", text_buf);

		counter++;

		vTaskDelay(pdMS_TO_TICKS(1000)); // Затримка 1 с
	}
}