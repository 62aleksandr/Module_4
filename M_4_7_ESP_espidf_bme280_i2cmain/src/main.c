// https://github.com/UncleRus/esp-idf-lib?utm_source=chatgpt.com
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <bmp280.h>
#include <i2cdev.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define I2C_PORT I2C_NUM_0
#define I2C_MASTER_SDA_IO 16
#define I2C_MASTER_SCL_IO 15

static const char *TAG = "MAIN_BME";

void app_main(void)
{
	esp_err_t err;
	i2c_master_bus_handle_t bus_handle;

	i2c_master_bus_config_t bus_cfg = {
		.i2c_port = I2C_PORT,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.flags.enable_internal_pullup = true,
	};

	err = i2c_new_master_bus(&bus_cfg, &bus_handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "I2C bus init failed");
		return;
	}

	i2c_dev_set_bus(bus_handle);

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

	ESP_LOGI(TAG, "handle = %p", bmp_dev.i2c_dev.handle);

	// 1. Створення та заповнення структури стандартних параметрів
	bmp280_params_t bmp_params;
	bmp280_init_default_params(&bmp_params);

	// 2. Апаратна ініціалізація датчика
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
		vTaskDelay(pdMS_TO_TICKS(1000));

		// Читання даних
		err = bmp280_read_float(&bmp_dev, &temperature, &pressure, &humidity);

		if (err != ESP_OK)
		{
			ESP_LOGE("BMP280", "Failed to read temperature %s", esp_err_to_name(err));
			continue;
		}

		ESP_LOGI("BMP280", "Temperature: %.2f C", temperature);
		ESP_LOGI("BMP280", "Humidity: %.2f %%", humidity);
		ESP_LOGI("BMP280", "Pressure: %.2f hPa", pressure);
	}
}
