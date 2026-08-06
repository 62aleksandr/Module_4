#include "i2c_master.h"

//------------- Налаштування I2C Master ---------------
esp_err_t i2c_init(i2c_mst_ctx_t *ctx)
{
	// Перевірка коректності аргументів
	if (ctx == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	if (ctx->initialized)
	{
		return ESP_ERR_INVALID_STATE;
	}

	//----1. Формування конфігурації I2C Master ---------------
	i2c_master_bus_config_t i2c_config = {};

	i2c_config.i2c_port = ctx->port;
	i2c_config.sda_io_num = ctx->sda_pin;
	i2c_config.scl_io_num = ctx->scl_pin;
	i2c_config.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_config.glitch_ignore_cnt = 7;
	i2c_config.flags.enable_internal_pullup = true;

	//----2. Ініціалізація I2C Master шини ---------------

	esp_err_t err = i2c_new_master_bus(&i2c_config, &ctx->bus_handle);

	if (err != ESP_OK)
	{
		return err;
	}

	ctx->initialized = true;

	return ESP_OK;
}

//------------- Додавання DEV пристрою ---------------
esp_err_t i2c_add_device(i2c_mst_ctx_t *ctx,
						 uint16_t dev_addr,
						 i2c_master_dev_handle_t *dev_handle)
{
	// Перевірка коректності аргументів
	if (ctx == NULL || dev_handle == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Проверка шины I2C
	if (!ctx->initialized || ctx->bus_handle == NULL)
	{
		return ESP_ERR_INVALID_STATE;
	}

	// Перевірка, чи пристрій вже створений
	if (*dev_handle != NULL)
	{
		return ESP_ERR_INVALID_STATE;
	}

	// Конфігурація I2C пристрою
	i2c_device_config_t dev_cfg = {};

	dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
	dev_cfg.device_address = dev_addr;
	dev_cfg.scl_speed_hz = ctx->clock_speed;

	// Додавання пристрою до I2C шини
	esp_err_t err = i2c_master_bus_add_device(ctx->bus_handle,
											  &dev_cfg, dev_handle);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

//------------- Передавання даних через I2C ---------------
esp_err_t i2c_write(i2c_master_dev_handle_t dev_handle,
					const uint8_t *data, size_t length)
{
	// Перевірка коректності аргументів
	if (dev_handle == NULL || data == NULL || length == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t err = i2c_master_transmit(dev_handle, data, length, -1);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

//------------- Читання даних через I2C ---------------
esp_err_t i2c_read(i2c_master_dev_handle_t dev_handle,
				   uint8_t *data, size_t length)
{
	// Перевірка коректності аргументів
	if (dev_handle == NULL || data == NULL || length == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Приймання даних через I2C
	esp_err_t err = i2c_master_receive(dev_handle, data, length, -1);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

//------------- Читання даних з регістру через I2C ---------------
esp_err_t i2c_read_register(i2c_master_dev_handle_t dev_handle,
							uint8_t reg_addr,
							uint8_t *data,
							size_t length)
{
	// Перевірка коректності аргументів
	if (dev_handle == NULL || data == NULL || length == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Передача адреси регістру та читання даних
	esp_err_t err = i2c_master_transmit_receive(dev_handle,
												&reg_addr,
												sizeof(reg_addr),
												data,
												length,
												-1);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

// Сканування I2C шини ---------------
//------------- Сканування I2C шини ---------------
esp_err_t i2c_scan(i2c_mst_ctx_t *ctx,
				   uint8_t *devices,
				   size_t max_devices,
				   size_t *found)
{
	// Перевірка коректності аргументів
	if (ctx == NULL || devices == NULL || found == NULL || max_devices == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Перевірка ініціалізації I2C шини
	if (!ctx->initialized || ctx->bus_handle == NULL)
	{
		return ESP_ERR_INVALID_STATE;
	}

	// Кількість знайдених пристроїв
	*found = 0;

	// Перебір I2C адрес
	for (uint8_t addr = 1; addr < 127; addr++)
	{
		esp_err_t err = i2c_master_probe(ctx->bus_handle,
										 addr,
										 100);

		if (err == ESP_OK)
		{
			// Збереження адреси знайденого пристрою
			if (*found < max_devices)
			{
				devices[*found] = addr;
				(*found)++;
			}
			else
			{
				return ESP_ERR_NO_MEM;
			}
		}
	}

	return ESP_OK;
}

//------------- Деініціалізація I2C ---------------
esp_err_t i2c_remove_devices(i2c_mst_ctx_t *ctx)
{
	if (ctx == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	if (ctx->devices.oled != NULL)
	{
		i2c_master_bus_rm_device(ctx->devices.oled);
		ctx->devices.oled = NULL;
	}

	if (ctx->devices.bme280 != NULL)
	{
		i2c_master_bus_rm_device(ctx->devices.bme280);
		ctx->devices.bme280 = NULL;
	}

	return ESP_OK;
}

esp_err_t i2c_deinit(i2c_mst_ctx_t *ctx)
{
	// Перевірка коректності аргументів
	if (ctx == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Перевірка стану I2C
	if (!ctx->initialized || ctx->bus_handle == NULL)
	{
		return ESP_ERR_INVALID_STATE;
	}

	// Видалення I2C шини
	esp_err_t err = i2c_del_master_bus(ctx->bus_handle);

	if (err != ESP_OK)
	{
		return err;
	}

	// Очищення handle шини
	ctx->bus_handle = NULL;

	// Стан I2C не ініціалізований
	ctx->initialized = false;

	return ESP_OK;
}
