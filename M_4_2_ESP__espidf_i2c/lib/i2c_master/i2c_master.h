#pragma once

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

typedef struct
{
	i2c_master_dev_handle_t oled;
	i2c_master_dev_handle_t bme280;

} i2c_devices_t;

typedef struct
{
	i2c_port_t port;					// Номер I2C порту
	gpio_num_t sda_pin;					// Пін SDA
	gpio_num_t scl_pin;					// Пін SCL
	uint32_t clock_speed;				// Частота I2C: 100000, 400000
	i2c_master_bus_handle_t bus_handle; // Handle шини I2C
	i2c_devices_t devices;
	bool initialized; // Стан ініціалізації

} i2c_mst_ctx_t;

esp_err_t i2c_init(i2c_mst_ctx_t *ctx);

esp_err_t i2c_add_device(i2c_mst_ctx_t *ctx,
						 uint16_t dev_addr,
						 i2c_master_dev_handle_t *dev_handle);

esp_err_t i2c_write(i2c_mst_ctx_t *ctx,
					uint8_t dev_addr,
					const uint8_t *data,
					size_t length);

esp_err_t i2c_read(i2c_mst_ctx_t *ctx,
				   uint8_t dev_addr,
				   uint8_t *data,
				   size_t length);

esp_err_t i2c_scan(i2c_mst_ctx_t *ctx);

esp_err_t i2c_remove_devices(i2c_mst_ctx_t *ctx);

esp_err_t i2c_deinit(i2c_mst_ctx_t *ctx);