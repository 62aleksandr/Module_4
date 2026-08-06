#pragma once

#include <esp_err.h>
#include <string.h>

typedef struct
{
	float temperature;
	float humidity;
	float pressure;
} bme280_data_t;

typedef struct
{
	bme280_data_t bme280_data;
	struct tm time;
} app_data_t;

app_data_t *get_app_data(void);

esp_err_t i2c_init();
// Ініціалізація OLED-дисплея
esp_err_t oled_dev_init();
// Ініціалізація RTC
esp_err_t rtc_dev_init(app_data_t *app_data);
// Ініціалізація BMP280
esp_err_t bmp280_dev_init();
// Читання часу з RTC
esp_err_t rtc_read(app_data_t *app_data);
// Читання даних з BME280
esp_err_t bme280_read(app_data_t *app_data);
// Оновлення OLED-дисплея
void oled_update(app_data_t *app_data);