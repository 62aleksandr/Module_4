#include "i2cdev.h"
#include <string.h>

static i2c_master_bus_handle_t bus_handle;

void i2c_dev_set_bus(i2c_master_bus_handle_t bus)
{
	bus_handle = bus;
}

esp_err_t i2c_dev_delete_mutex(i2c_dev_t *dev)
{
	return ESP_OK;
}

esp_err_t i2c_dev_create_mutex(i2c_dev_t *dev)
{
	if (!dev)
		return ESP_ERR_INVALID_ARG;

	if (bus_handle == NULL)
		return ESP_ERR_INVALID_STATE;

	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = dev->addr,
		.scl_speed_hz = dev->cfg.master.clk_speed,
	};

	return i2c_master_bus_add_device(
		bus_handle,
		&dev_cfg,
		&dev->handle);
}

esp_err_t i2c_dev_read_reg(i2c_dev_t *dev,
						   uint8_t reg,
						   void *data,
						   size_t len)
{
	return i2c_master_transmit_receive(
		dev->handle,
		&reg,
		1,
		data,
		len,
		-1);
}

esp_err_t i2c_dev_write_reg(i2c_dev_t *dev,
							uint8_t reg,
							const void *data,
							size_t len)
{
	uint8_t buffer[32];

	if (len + 1 > sizeof(buffer))
	{
		return ESP_ERR_INVALID_SIZE;
	}

	// перший байт - адреси регистра
	buffer[0] = reg;

	memcpy(&buffer[1], data, len);

	return i2c_master_transmit(
		dev->handle,
		buffer,
		sizeof(buffer),
		-1);
}

esp_err_t i2c_dev_read(
	const i2c_dev_t *dev,
	const void *out_data,
	size_t out_size,
	void *in_data,
	size_t in_size)
{
	return i2c_master_transmit_receive(
		dev->handle,
		out_data,
		out_size,
		in_data,
		in_size,
		-1);
}
