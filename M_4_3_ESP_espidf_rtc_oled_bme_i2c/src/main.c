// https://github.com/UncleRus/esp-idf-lib?utm_source=chatgpt.com

#include <stdio.h>
#include <freertos/FreeRTOS.h>

#include <freertos/task.h>
#include <esp_log.h>
#include <string.h>
#include <bmp280.h>
#include <i2cdev.h>
#include "ssd1306.h"
#include "ds1307.h"

// Конфигурация I2C для вашего ESP32-S3
#define I2C_PORT I2C_NUM_0
#define I2C_MASTER_SDA_IO 16 // Укажите ваш реальный GPIO для SDA
#define I2C_MASTER_SCL_IO 15 // Укажите ваш реальный GPIO для SCL

static const char *TAG = "MAIN";

// Ініціалізація поточеого часу
static void rtc_set_time(struct tm *time)
{

	static const char *months[] =
		{
			"Jan", "Feb", "Mar", "Apr",
			"May", "Jun", "Jul", "Aug",
			"Sep", "Oct", "Nov", "Dec"};

	char month[4];
	int year;
	memset(time, 0, sizeof(struct tm));

	sscanf(__DATE__, "%3s %d %d",
		   month,
		   &time->tm_mday,
		   &year);

	time->tm_year = year - 1900;

	for (int i = 0; i < 12; i++)
	{
		if (strcmp(month, months[i]) == 0)
		{
			time->tm_mon = i;
			break;
		}
	}

	sscanf(__TIME__, "%d:%d:%d",
		   &time->tm_hour,
		   &time->tm_min,
		   &time->tm_sec);
}

void app_main(void)
{
	struct tm time;
	esp_err_t err;

	// ----- Int i2cdev------
	err = i2cdev_init();
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "i2cdev_init() failed: %s", esp_err_to_name(err));
		return;
	}

	// --------- OLED -----------------
	// Make handle OLED
	ssd1306_t oled_dev;
	memset(&oled_dev, 0, sizeof(ssd1306_t));

	err = ssd1306_init_desc(&oled_dev, I2C_PORT,
							I2C_MASTER_SDA_IO,
							I2C_MASTER_SCL_IO);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init descriptor: %d", err);
		return;
	}

	err = ssd1306_init_display(&oled_dev);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init display: %d", err);
		return;
	}

	// Очистка экрана перед выводом информации
	ssd1306_clear(&oled_dev);

	// // Вывод статического заголовка (на 0-ю страницу экрана)
	// ssd1306_draw_string(&oled_dev, 0, 0, "ESP32-S3 OLED");
	// ssd1306_draw_string(&oled_dev, 0, 1, "SYSTEM READY");

	char text_buf[32];
	uint32_t counter = 0;

	// --------- RTC -----------------
	// Make handle RTC
	i2c_dev_t rtc_dev;
	memset(&rtc_dev, 0, sizeof(i2c_dev_t));

	err = ds1307_init_desc(&rtc_dev, I2C_PORT,
						   I2C_MASTER_SDA_IO,
						   I2C_MASTER_SCL_IO);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "RTC_init_desc: %s", esp_err_to_name(err));
		return;
	}

	// Установка времени на RTC
	rtc_set_time(&time);

	err = ds1307_set_time(&rtc_dev, &time);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "RTC_set_time: %s", esp_err_to_name(err));
		return;
	}

	// --------- BME280 -----------------
	bmp280_t bmp_dev;
	memset(&bmp_dev, 0, sizeof(bmp280_t));

	// Створення та ініціалізація дескриптора BME280
	err = bmp280_init_desc(&bmp_dev, BMP280_I2C_ADDRESS_0, I2C_PORT, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "bmp280_init_desc: %s", esp_err_to_name(err));
		bmp280_free_desc(&bmp_dev);
		return;
	}

	// Создаем и заполняем структуру стандартных параметров
	bmp280_params_t bmp_params;
	bmp280_init_default_params(&bmp_params);

	// Запускаем аппаратную инициализацию датчика
	err = bmp280_init(&bmp_dev, &bmp_params);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "BMP280 init failed: %s", esp_err_to_name(err));
		return;
	}

	// Read temperature, humidity and pressure
	float temperature = 0.0f, humidity = 0.0f, pressure = 0.0f;

	while (1)
	{
		// Читання часу
		err = ds1307_get_time(&rtc_dev, &time);
		if (err != ESP_OK)
		{
			ESP_LOGE(TAG, "ds1307_get_time: %s", esp_err_to_name(err));
			return;
		}

		ESP_LOGI(TAG,
				 "%04d-%02d-%02d %02d:%02d:%02d\n",
				 time.tm_year + 1900 /*Add 1900 for better readability*/,
				 time.tm_mon + 1,
				 time.tm_mday,
				 time.tm_hour,
				 time.tm_min,
				 time.tm_sec);

		// Считываем все данные одной функцией (передаем указатель &bmp_dev)
		err = bmp280_read_float(&bmp_dev, &temperature, &pressure, &humidity);

		if (err != ESP_OK)
		{
			ESP_LOGE("BMP280", "Failed to read temperature %s", esp_err_to_name(err));
			continue;
		}

		ESP_LOGI("BMP280", "Temperature: %.2f C", temperature);
		ESP_LOGI("BMP280", "Humidity: %.2f %%", humidity);
		ESP_LOGI("BMP280", "Pressure: %.2f hPa", pressure);

		// Формируем дату
		snprintf(text_buf, sizeof(text_buf),
				 "%02d.%02d.%04d",
				 time.tm_mday,
				 time.tm_mon + 1,
				 time.tm_year + 1900);

		ssd1306_draw_string(&oled_dev, 5, 1, text_buf);

		// Формируем час
		snprintf(text_buf, sizeof(text_buf),
				 "%02d:%02d:%02d",
				 time.tm_hour,
				 time.tm_min,
				 time.tm_sec);

		ssd1306_draw_string(&oled_dev, 5, 2, text_buf);

		// Температура
		snprintf(text_buf, sizeof(text_buf),
				 "T: %.1f C",
				 temperature);

		ssd1306_draw_string(&oled_dev, 5, 4, text_buf);

		// Влажность
		snprintf(text_buf, sizeof(text_buf),
				 "H: %.1f %%",
				 humidity);

		ssd1306_draw_string(&oled_dev, 5, 5, text_buf);

		// Давление
		snprintf(text_buf, sizeof(text_buf),
				 "P: %.1f hPa",
				 pressure);

		ssd1306_draw_string(&oled_dev, 5, 6, text_buf);

		vTaskDelay(pdMS_TO_TICKS(1000)); // Задержка в 1 секунду
	}
}
