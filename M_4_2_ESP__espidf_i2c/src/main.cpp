#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "i2c_master.h"
#include "bme280.h"

static constexpr i2c_port_t i2c_port = I2C_NUM_1;
static constexpr gpio_num_t i2c_sda_pin = GPIO_NUM_8;
static constexpr gpio_num_t i2c_scl_pin = GPIO_NUM_9;
static constexpr uint32_t i2c_speed_std = 100000; // Частота I2C

static constexpr uint16_t oled_i2c_addr = 0x3C;	  // Адрес OLED
static constexpr uint16_t bme280_i2c_addr = 0x76; // Адрес OLED

static const char *TAG = "MAIN";

// Обертка для чтения из регистра датчика
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

// Обертка для записи в регистр датчика
int8_t bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	auto dev_handle = static_cast<i2c_master_dev_handle_t>(intf_ptr);

	// Формируем буфер: [адрес регистра, данные0, данные1...]
	uint8_t *buffer = (uint8_t *)malloc(len + 1);
	if (buffer == NULL)
		return -1;

	buffer[0] = reg_addr;
	for (size_t i = 0; i < len; i++)
	{
		buffer[i + 1] = reg_data[i];
	}

	// Вызываем вашу готовую функцию i2c_write
	esp_err_t err = i2c_write(dev_handle, buffer, len + 1);
	free(buffer);

	return (err == ESP_OK) ? 0 : -1;
}

// Обертка для задержки
void bme280_delay_us(uint32_t period, void *intf_ptr)
{
	vTaskDelay(pdMS_TO_TICKS(period / 1000 + 1));
}

extern "C" void app_main()
{
	//------------ I2C CONTEXT Init ---------------
	static i2c_mst_ctx_t i2c_mst_ctx = {};

	i2c_mst_ctx.port = i2c_port;			 // Номер I2C контролера
	i2c_mst_ctx.sda_pin = i2c_sda_pin;		 // Пін SDA
	i2c_mst_ctx.scl_pin = i2c_scl_pin;		 // Пін SCL
	i2c_mst_ctx.clock_speed = i2c_speed_std; // Частота I2C: 100 кГц

	i2c_mst_ctx.initialized = false; // Стан ініціалізації

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
		ESP_LOGE(TAG, "i2c_add_device: %s", esp_err_to_name(err));
	}
	//------------ ADD BME280 ---------------
	err = i2c_add_device(&i2c_mst_ctx,
						 bme280_i2c_addr,
						 &i2c_mst_ctx.devices.bme280);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "i2c_add_device: %s", esp_err_to_name(err));
	}

	//================== ДОБАВЛЯТЬ СЮДА ==================

	// 1. Создаем структуру датчика из вашей библиотеки (инициализирует встроенный I2C)
	bme280_t bme_dev = {};
	bme_dev.i2c_dev.port = i2c_port;
	bme_dev.i2c_dev.addr = bme280_i2c_addr;
	bme_dev.i2c_dev.cfg.sda_io_num = i2c_sda_pin;
	bme_dev.i2c_dev.cfg.scl_io_num = i2c_scl_pin;
	bme_dev.i2c_dev.cfg.master.clk_speed = i2c_speed_std;

	// Инициализируем внутренние параметры драйвера
	if (bme280_init_desc(&bme_dev) != ESP_OK)
	{
		ESP_LOGE(TAG, "BME280 init descriptor FAILED!");
	}

	// Настраиваем конфигурацию (Normal mode и оверсэмплинг)
	bme280_params_t bme_params;
	bme280_get_default_params(&bme_params);
	bme_params.mode = BME280_MODE_NORMAL;
	bme_params.osr_h = BME280_SAMPLING_X1;	// Исправлено
	bme_params.osr_p = BME280_SAMPLING_X16; // Исправлено
	bme_params.osr_t = BME280_SAMPLING_X2;	// Исправлено
	bme_params.filter = BME280_FILTER_16;

	// 2. Инициализируем сам чип BME280
	esp_err_t bme_rslt = bme280_init(&bme_dev, &bme_params);
	if (bme_rslt != ESP_OK)
	{
		ESP_LOGE(TAG, "BME280 Init FAILED! Code: %d", bme_rslt);
	}
	else
	{
		ESP_LOGI(TAG, "BME280 Init SUCCESS");
	}

	// 3. Основной цикл опроса
	float temperature, pressure, humidity;

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(2000)); // Опрос каждые 2 секунды

		if (bme_rslt == BME280_OK)
		{
			if (bme280_get_sensor_data(BME280_ALL, &bme_data, &bme_dev) == BME280_OK)
			{
				ESP_LOGI(TAG, "Temp: %.2f C | Press: %.2f Pa | Hum: %.2f %%",
						 bme_data.temperature, bme_data.pressure, bme_data.humidity);
			}
			else
			{
				ESP_LOGE(TAG, "Ошибка чтения данных BME280");
			}
		}
	}
}