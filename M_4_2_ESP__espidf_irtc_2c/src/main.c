#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SDA_IO 16
#define I2C_MASTER_SCL_IO 15
#define I2C_MASTER_NUM 0
#define DS1307_SENSOR_ADDR 0x68
#define DS1307_REG_YEAR 0x06

static const char *TAG = "DS1307_I2C";

// Конвертація з Decimal у BCD
uint8_t dec_to_bcd(uint8_t val)
{
	return ((val / 10) << 4) + (val % 10);
}

// Конвертація з BCD у Decimal
uint8_t bcd_to_dec(uint8_t val)
{
	return ((val >> 4) * 10) + (val & 0x0F);
}

void app_main(void)
{
	// 1. Конфігурація шини I2C (Master Bus)
	i2c_master_bus_config_t bus_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = I2C_MASTER_NUM,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};
	i2c_master_bus_handle_t bus_handle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

	// 2. Конфігурація пристрою на шині (DS1307)
	i2c_device_config_t dev_config = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = DS1307_SENSOR_ADDR,
		.scl_speed_hz = 100000,
	};
	i2c_master_dev_handle_t dev_handle;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

	ESP_LOGI(TAG, "I2C успішно ініціалізован.");

	// 3. Запись року в DS1307
	uint8_t year_to_write = 26;
	uint8_t write_buf[2] = {DS1307_REG_YEAR, dec_to_bcd(year_to_write)};

	// Передача адреси регістру, а потім дані
	esp_err_t err = i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), -1);
	if (err == ESP_OK)
	{
		ESP_LOGI(TAG, "Успішно записан рік: 20%02d", year_to_write);
	}
	else
	{
		ESP_LOGE(TAG, "Помилка запису: %s", esp_err_to_name(err));
	}

	// Пауза
	vTaskDelay(pdMS_TO_TICKS(100));

	// 4. Читаємо рік з DS1307
	uint8_t reg_addr = DS1307_REG_YEAR;
	uint8_t raw_year_received = 0;

	// Спочатку адресу регістру, потім читаємо з регистру 1 байт
	err = i2c_master_transmit_receive(dev_handle, &reg_addr, 1, &raw_year_received, 1, -1);
	if (err == ESP_OK)
	{
		uint8_t final_year = bcd_to_dec(raw_year_received);
		ESP_LOGI(TAG, "Успішно прочитан рік з RTC: 20%02d", final_year);
	}
	else
	{
		ESP_LOGE(TAG, "Помилка читання: %s", esp_err_to_name(err));
	}

	// Очистка ресурсів або дешнівання
	i2c_master_bus_rm_device(dev_handle);
	i2c_del_master_bus(bus_handle);
}