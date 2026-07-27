#include "uart_drv.h"

HAL_StatusTypeDef uart_init(uart_context *ctx,
							UART_HandleTypeDef *huart)
{
	if (ctx == NULL || huart == NULL)
	{
		return HAL_ERROR;
	}

	ctx->huart = huart;
	ctx->initialized = 1;

	return HAL_OK;
}

HAL_StatusTypeDef uart_send(uart_context *ctx,
							uint8_t *data,
							uint16_t length)
{

	if (ctx == NULL ||
		ctx->initialized == 0 ||
		data == NULL)
	{
		return HAL_ERROR;
	}

	return HAL_UART_Transmit(ctx->huart,
							 data,
							 length,
							 100);
}

HAL_StatusTypeDef uart_receive(uart_context *ctx,
							   uint8_t *data,
							   uint16_t length,
							   uint32_t timeout)
{

	if (ctx == NULL ||
		ctx->initialized == 0 ||
		data == NULL)
	{
		return HAL_ERROR;
	}

	return HAL_UART_Receive(ctx->huart,
							data,
							length,
							timeout);
}
