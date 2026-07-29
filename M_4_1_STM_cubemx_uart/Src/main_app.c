#include <string.h>
#include "uart_drv.h"
#include "main.h"

extern UART_HandleTypeDef huart2;

// UART_HandleTypeDef *huart2;

#define UART_BUFFER_SIZE 1 // Розмір буфера для прийому даних з UART
uint8_t rxData[UART_BUFFER_SIZE];

void app_init(uart_context *uart_ctx, UART_HandleTypeDef *huart)
{
	uart_init(uart_ctx, huart);
}

void app_run(uart_context *uart_ctx)
{
	if (uart_receive(uart_ctx,
					 rxData,
					 UART_BUFFER_SIZE,
					 0) == HAL_OK)
	{
		uart_send(uart_ctx,
				  rxData,
				  UART_BUFFER_SIZE);

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	}
}

void main_app()
{
	uart_context uart2_ctx;
	// huart2 = uart_get_handle();
	app_init(&uart2_ctx, &huart2);

	while (1)
	{
		app_run(&uart2_ctx);
	}
}
