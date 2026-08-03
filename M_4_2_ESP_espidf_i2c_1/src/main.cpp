#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
// #include "i2c_master.h" // Ваш локальный заголовочный файл драйвера
#include "bme280.h" // Заголовочный файл Bosch API

static constexpr i2c_port_t i2c_port = I2C_NUM_1;
static constexpr gpio_num_t i2c_sda_pin = GPIO_NUM_8;
static constexpr gpio_num_t i2c_scl_pin = GPIO_NUM_9;
static constexpr uint32_t i2c_speed_std = 100000; // Частота I2C: 100 кГц

static constexpr uint16_t oled_i2c_addr = 0x3C;	  // Адрес OLED
static constexpr uint16_t bme280_i2c_addr = 0x76; // Адрес BME280

static const char *TAG = "MAIN";

// =========================================================================
// ШАГ 1: Функции-обертки для связи Bosch API с вашим драйвером I2C
// =========================================================================

// Функция чтения для Bosch API
int8_t bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	auto dev_handle = static_cast<i2c_master_dev_handle_t>(intf_ptr);

	// Передаем адрес регистра и читаем данные за одну транзакцию
	if (i2c_master_transmit_receive(dev_handle, &reg_addr, 1, reg_data, len, -1) == ESP_OK)
	{
		return 0; // Успех для Bosch API
	}
	return -1; // Ошибка
}

// Функция записи для Bosch API
int8_t bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	auto dev_handle = static_cast<i2c_master_dev_handle_t>(intf_ptr);

	// Выделяем буфер под [адрес регистра + данные]
	uint8_t *buffer = (uint8_t *)malloc(len + 1);
	if (buffer == NULL)
		return -1;

	buffer[0] = reg_addr;
	for (size_t i = 0; i < len; i++)
	{
		buffer[i + 1] = reg_data[i];
	}

	// Вызываем вашу функцию записи из i2c_master.h
	esp_err_t err = i2c_write(dev_handle, buffer, len + 1);
	free(buffer);

	return (err == ESP_OK) ? 0 : -1;
}

// Функция задержки для Bosch API
void bme280_delay_us(uint32_t period, void *intf_ptr)
{
	vTaskDelay(pdMS_TO_TICKS(period / 1000 + 1));
}

// =========================================================================
// ГЛАВНАЯ ФУНКЦИЯ ПРИЛОЖЕНИЯ
// =========================================================================
extern "C" void app_main()
{
	//------------ I2C CONTEXT Init ---------------
	static i2c_mst_ctx_t i2c_mst_ctx = {};

	i2c_mst_ctx.port = i2c_port;			 // Номер I2C контролера
	i2c_mst_ctx.sda_pin = i2c_sda_pin;		 // Пін SDA
	i2c_mst_ctx.scl_pin = i2c_scl_pin;		 // Пін SCL
	i2c_mst_ctx.clock_speed = i2c_speed_std; // Частота I2C: 100 кГц
	i2c_mst_ctx.initialized = false;		 // Стан ініціалізації

	esp_err_t err = i2c_init(&i2c_mst_ctx);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "i2c_init: %s", esp_err_to_name(err));
	}
	ESP_LOGI(TAG, "I2C initialized");

	//------------ ADD OLED ---------------
	err = i2c_add_device(&i2c_mst_ctx,
						 oled_i2c_addr,
						 &i2c_mst_ctx.devices.oled);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "i2c_add_device (OLED): %s", esp_err_to_name(err));
	}

	//------------ ADD BME280 ---------------
	err = i2c_add_device(&i2c_mst_ctx,
						 bme280_i2c_addr,
						 &i2c_mst_ctx.devices.bme280);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "i2c_add_device (BME280): %s", esp_err_to_name(err));
	}

	// =========================================================================
	// ШАГ 2: Настройка структуры Bosch API и инициализация чипа
	// =========================================================================
	struct bme280_dev bme_dev;
	bme_dev.intf = BME280_I2C_INTF;
	bme_dev.read = bme280_i2c_read;
	bme_dev.write = bme280_i2c_write;
	bme_dev.delay_us = bme280_delay_us;

	// Передаем хэндл зарегистрированного I2C устройства напрямую
	bme_dev.intf_ptr = i2c_mst_ctx.devices.bme280;

	// bme280_init сама проверяет Chip ID и вычитывает заводскую калибровку
	int8_t bme_rslt = bme280_init(&bme_dev);
	if (bme_rslt != BME280_OK)
	{
		ESP_LOGE(TAG, "Датчик BME280 не найден! Код ошибки: %d", bme_rslt);
	}
	else
	{
		ESP_LOGI(TAG, "Датчик BME280 успешно найден и калиброван!");

		// Человекочитаемая конфигурация (заменяет ручную запись масок в регистры)
		bme_dev.settings.osr_h = BME280_OVERSAMPLING_1X;   // Влажность x1
		bme_dev.settings.osr_p = BME280_OVERSAMPLING_1X;   // Давление x1
		bme_dev.settings.osr_t = BME280_OVERSAMPLING_1X;   // Температура x1
		bme_dev.settings.filter = BME280_FILTER_COEFF_OFF; // Без фильтрации

		// Применяем настройки параметров выборки
		uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
		bme280_set_sensor_settings(settings_sel, &bme_dev);

		// Переводим датчик в Normal Mode (режим постоянного измерения)
		bme280_set_sensor_mode(BME280_NORMAL_MODE, &bme_dev);

		ESP_LOGI(TAG, "Старт бесконечного цикла чтения данных...");
	}

	// =========================================================================
	// ШАГ 3: Бесконечный цикл чтения откомпенсированных данных
	// =========================================================================
	struct bme280_data bme_data;

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(2000)); // Опрос датчика каждые 2 секунды

		if (bme_rslt == BME280_OK)
		{
			// Функция сама берет сырые данные и считает их по калибровочным таблицам
			if (bme280_get_sensor_data(BME280_ALL, &bme_data, &bme_dev) == BME280_OK)
			{
				ESP_LOGI(TAG, "Temp: %.2f C | Press: %.2f Pa | Hum: %.2f %%",
						 bme_data.temperature, bme_data.pressure, bme_data.humidity);
			}
			else
			{
				ESP_LOGE(TAG, "Ошибка обмена данными с BME280");
			}
		}
	}
}