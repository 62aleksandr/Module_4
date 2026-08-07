#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct
{
	i2c_master_dev_handle_t handle;

	uint8_t addr;

	int port;

	struct
	{
		struct
		{
			uint32_t clk_speed;
		} master;

		int sda_io_num;
		int scl_io_num;

	} cfg;

} i2c_dev_t;

void i2c_dev_set_bus(i2c_master_bus_handle_t bus);

esp_err_t i2c_dev_create_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_delete_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_read_reg(i2c_dev_t *dev,
						   uint8_t reg,
						   void *data,
						   size_t len);

esp_err_t i2c_dev_write_reg(i2c_dev_t *dev,
							uint8_t reg,
							const void *data,
							size_t len);

esp_err_t i2c_dev_read(
	const i2c_dev_t *dev,
	const void *out_data,
	size_t out_size,
	void *in_data,
	size_t in_size);