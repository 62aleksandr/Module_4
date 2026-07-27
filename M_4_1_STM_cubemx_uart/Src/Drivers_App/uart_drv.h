#pragma once

#include "stm32f4xx_hal.h"

#define UART_TIMEOUT_MS 100 // Таймаут для UART в мілісекундах

typedef struct
{
	UART_HandleTypeDef *huart;

	uint8_t initialized;

} uart_context;

// Инициализация контекста UART
HAL_StatusTypeDef uart_init(uart_context *ctx,
							UART_HandleTypeDef *huart);

// Передача данных
HAL_StatusTypeDef uart_send(uart_context *ctx,
							uint8_t *data,
							uint16_t length);

// Прием данных
HAL_StatusTypeDef uart_receive(uart_context *ctx,
							   uint8_t *data,
							   uint16_t length,
							   uint32_t timeout);
