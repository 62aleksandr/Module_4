// https://github.com/UncleRus/esp-idf-lib?utm_source=chatgpt.com

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include <esp_log.h>

#include "app.h"

static const char *TAG = "MAIN";

void app_main(void)
{

	// Вказівник на структуру app_data_t
	app_data_t *data = get_app_data();

	// Iніціалізацію бібліотеки i2cdev
	esp_err_t err = i2c_init();
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "i2c_init: %s", esp_err_to_name(err));
	}
	// Ініціалізація OLED-дисплея
	err = oled_dev_init();
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "oled_init: %s", esp_err_to_name(err));
	}
	// Ініціалізація RTC
	err = rtc_dev_init(data);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "rtc_dev_init: %s", esp_err_to_name(err));
	}
	// Ініціалізація BMP280
	err = bmp280_dev_init();
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "bmp280_dev_init: %s", esp_err_to_name(err));
	}

	while (1)
	{
		// Читання часу з RTC
		rtc_read(data);

		// Читання даних з BME280
		bme280_read(data);

		// Оновлення OLED-дисплея
		oled_update(data);

		ESP_LOGI(TAG, "Time: %04d-%02d-%02d %02d:%02d:%02d",
				 data->time.tm_year + 1900,
				 data->time.tm_mon + 1,
				 data->time.tm_mday,
				 data->time.tm_hour,
				 data->time.tm_min,
				 data->time.tm_sec);

		ESP_LOGI(TAG, "T: %.2f C H: %.2f %% P: %.2f hPa",
				 data->bme280_data.temperature,
				 data->bme280_data.humidity,
				 data->bme280_data.pressure);

		vTaskDelay(pdMS_TO_TICKS(1000)); // Задержка в 1 секунду
	}
}
