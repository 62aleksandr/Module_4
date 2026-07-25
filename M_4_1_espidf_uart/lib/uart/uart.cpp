#include "uart.h"

//------------- Налаштування  UART  -------
esp_err_t uart_init(uart_context *ctx)
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

	//----1. Формування conf UART ---------------
	uart_config_t uart_config = {};

	uart_config.baud_rate = ctx->baud_rate;
	uart_config.data_bits = ctx->data_bits;
	uart_config.parity = ctx->parity;
	uart_config.stop_bits = ctx->stop_bits;
	uart_config.flow_ctrl = ctx->flow_ctrl;
	uart_config.source_clk = UART_SCLK_DEFAULT;

	//----2. Налаштування параметрів UART ---------------
	esp_err_t err = uart_param_config(ctx->uart_num, &uart_config);

	if (err != ESP_OK)
	{
		return err;
	}

	//----3. Установка GPIO для UART ---------------
	err = uart_set_pin(ctx->uart_num, ctx->tx_pin, ctx->rx_pin,
					   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	if (err != ESP_OK)
	{
		return err;
	}

	//----4. Установка драйвера UART ---------------
	err = uart_driver_install(ctx->uart_num, ctx->rx_buffer_size,
							  ctx->tx_buffer_size, 0, NULL, 0);

	if (err != ESP_OK)
	{
		return err;
	}

	ctx->initialized = true;
	return ESP_OK;
}

//------------- Передавання даних через UART ---------------
esp_err_t uart_send(uart_context *ctx,
					const uint8_t *data,
					size_t length)
{
	// Перевірка коректності аргументів
	if (ctx == NULL || data == NULL || length == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Перевірка ініціалізації UART
	if (!ctx->initialized)
	{
		return ESP_ERR_INVALID_STATE;
	}

	// Передавання даних
	int bytes_sent = uart_write_bytes(ctx->uart_num,
									  data,
									  length);

	// Перевірка результату передачі
	if (bytes_sent < 0)
	{
		return ESP_FAIL;
	}

	// Перевірка кількості переданих байтів
	if ((size_t)bytes_sent != length)
	{
		return ESP_FAIL;
	}

	return ESP_OK;
}

//------------- Прийом даних по UART ---------------
esp_err_t uart_receive(uart_context *ctx,
					   uint8_t *buffer,
					   size_t length,
					   TickType_t timeout,
					   size_t *received)
{
	// Перевірка коректності аргументів
	if (ctx == NULL || buffer == NULL || received == NULL || length == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Перевірка ініціалізації UART
	if (!ctx->initialized)
	{
		return ESP_ERR_INVALID_STATE;
	}

	// Прийом даних з UART RX буфера
	int bytes_received = uart_read_bytes(ctx->uart_num,
										 buffer,
										 length,
										 timeout);

	// Перевірка результату прийому
	if (bytes_received < 0)
	{
		*received = 0;
		return ESP_FAIL;
	}

	// Кількість прийнятих байтів
	*received = (size_t)bytes_received;

	return ESP_OK;
}

//------------- Перевірка наявності даних UART ---------------
esp_err_t uart_available(uart_context *ctx,
						 size_t *length)
{
	// Перевірка коректності аргументів
	if (ctx == NULL || length == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Отримання кількості прийнятих байт у RX-буфері
	esp_err_t err = uart_get_buffered_data_len(ctx->uart_num, length);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

//------------- Очищення RX буфера UART ---------------
esp_err_t uart_flush_rx(uart_context *ctx)
{
	// Перевірка коректності аргументів
	if (ctx == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Очищення приймального буфера RX
	esp_err_t err = uart_flush_input(ctx->uart_num);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

//------------- Деініціалізація UART ---------------
esp_err_t uart_deinit(uart_context *ctx)
{
	// Перевірка коректності аргументів
	if (ctx == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	// Перевірка, чи UART вже деініціалізований
	if (!ctx->initialized)
	{
		return ESP_OK;
	}

	// Видалення драйвера UART та звільнення ресурсів
	esp_err_t err = uart_driver_delete(ctx->uart_num);

	// UART - неініціалізований
	if (err == ESP_OK)
	{
		ctx->initialized = false;
	}

	return err;
}
